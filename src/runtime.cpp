#include "framework.h"

constexpr auto sleep_time = 100;

bool FindOffsets()
{
	auto pOffsets = std::make_unique<COffsets>();

	auto time_counter = std::chrono::milliseconds::duration(0);

	while (true)
	{
		::Sleep(sleep_time);

		time_counter += std::chrono::milliseconds(sleep_time);

		if (time_counter > std::chrono::seconds::duration(10))
			return false;

		if (!pOffsets->FindModules())
			continue;

		if (!pOffsets->FindInterfaces())
			continue;

		break;
	}

	return pOffsets->FindGameOffsets();
}

bool EntryPoint()
{
	if (!FindOffsets())
		return false;

	return SetupHooks();
}


#ifdef _WIN64
#define CTX_IP Rip
#define CTX_SP Rsp
#else
#define CTX_IP Eip
#define CTX_SP Esp
#endif

// True only if every other thread is provably outside this module: IP not inside it and no live stack
// slot (SP upward, bounded by the stack allocation) holds an address inside it. Conservative: a stale
// pointer in a live frame gives a false "busy", never a false "idle". Nothing allocates or logs while
// threads are suspended.
static bool NoThreadIsInsideModule(HMODULE module)
{
	MODULEINFO mi{};

	if (!GetModuleInformation(GetCurrentProcess(), module, &mi, sizeof(mi)))
		return false;

	const uintptr_t lo = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
	const uintptr_t hi = lo + mi.SizeOfImage;
	const DWORD self_tid = GetCurrentThreadId();

	std::vector<HANDLE> threads;
	threads.reserve(512);

	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (snap == INVALID_HANDLE_VALUE)
		return false;

	THREADENTRY32 te{ sizeof(te) };
	bool open_ok = true;

	for (BOOL more = Thread32First(snap, &te); more; more = Thread32Next(snap, &te))
	{
		if (te.th32OwnerProcessID != GetCurrentProcessId() || te.th32ThreadID == self_tid)
			continue;

		HANDLE h = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, te.th32ThreadID);

		if (!h || threads.size() == threads.capacity())
		{
			if (h)
				CloseHandle(h);

			open_ok = false;
			break;
		}

		threads.push_back(h);
	}

	CloseHandle(snap);

	bool idle = open_ok;
	size_t suspended = 0;

	if (idle)
	{
		for (HANDLE h : threads)
		{
			if (SuspendThread(h) == static_cast<DWORD>(-1))
			{
				idle = false;
				break;
			}

			suspended++;
		}
	}

	for (size_t i = 0; idle && i < suspended; i++)
	{
		CONTEXT ctx{};
		ctx.ContextFlags = CONTEXT_CONTROL;

		if (!GetThreadContext(threads[i], &ctx))
		{
			idle = false;
			break;
		}

		const uintptr_t ip = static_cast<uintptr_t>(ctx.CTX_IP);
		const uintptr_t sp = static_cast<uintptr_t>(ctx.CTX_SP);

		if (ip >= lo && ip < hi)
		{
			idle = false;
			break;
		}

		MEMORY_BASIC_INFORMATION mbi{};

		if (!VirtualQuery(reinterpret_cast<LPCVOID>(sp), &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
		{
			idle = false;
			break;
		}

		const uintptr_t stack_end = (std::min<uintptr_t>)(reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize, sp + 0x100000);

		for (uintptr_t p = sp & ~(sizeof(uintptr_t) - 1); p + sizeof(uintptr_t) <= stack_end; p += sizeof(uintptr_t))
		{
			const uintptr_t value = *reinterpret_cast<const uintptr_t*>(p);

			if (value >= lo && value < hi)
			{
				idle = false;
				break;
			}
		}
	}

	for (size_t i = 0; i < suspended; i++)
		ResumeThread(threads[i]);

	for (HANDLE h : threads)
		CloseHandle(h);

	return idle;
}

// Waits for the named event set by the injector tool. Returns only if the unload was cancelled
// (render thread never serviced it, or no quiet moment was found); the DLL then stays loaded.
static void ServeUnloadRequests(HANDLE hUnloadEvent)
{
	const HMODULE self = g_pGlobals->m_hModule;

	for (;;)
	{
		WaitForSingleObject(hUnloadEvent, INFINITE);
		ResetEvent(hUnloadEvent);

		g_pGlobals->m_bUnloadHooksRemoved = false;
		g_pGlobals->m_bUnloadRequested = true;

		// Hooks are removed on the render thread; give it a few seconds of frames.
		bool started = false;

		for (int i = 0; i < 200 && !started; i++)
		{
			Sleep(25);
			started = !g_pGlobals->m_bUnloadRequested;
		}

		if (!started)
		{
			bool expected = true;

			if (g_pGlobals->m_bUnloadRequested.compare_exchange_strong(expected, false))
				continue;
		}

		for (int i = 0; i < 400 && !g_pGlobals->m_bUnloadHooksRemoved; i++)
			Sleep(25);

		if (!g_pGlobals->m_bUnloadHooksRemoved)
			return;

		// Hooks are gone; the render thread may still be returning through our swap hook frame.
		bool idle = false;

		for (int i = 0; i < 200 && !idle; i++)
		{
			idle = NoThreadIsInsideModule(self);

			if (!idle)
				Sleep(25);
		}

		if (!idle)
			return;

		MH_Uninitialize();
		FreeLibraryAndExitThread(self, 0);
	}
}

static DWORD WINAPI MainThread(LPVOID hInstance)
{
	if (EntryPoint())
	{
		char name[64];
		sprintf_s(name, "Local\\pasteware_unload_%lu", GetCurrentProcessId());

		HANDLE hUnloadEvent = CreateEventA(nullptr, TRUE, FALSE, name);

		if (hUnloadEvent)
			ServeUnloadRequests(hUnloadEvent);

		// No unload handle, or unload could not be completed safely: stay loaded.
		for (;;)
			Sleep(1000);
	}

	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
			return FALSE;

		g_pGlobals = std::make_unique<CGlobals>();

		if (!g_pGlobals->m_hWnd || g_pGlobals->m_hWnd == INVALID_HANDLE_VALUE)
			return FALSE;

		g_pGlobals->m_hModule = hinstDLL;

		DisableThreadLibraryCalls(hinstDLL);

		HANDLE hMainThread = CreateThread(nullptr, 0, MainThread, hinstDLL, 0, nullptr);

		if (hMainThread)
			CloseHandle(hMainThread);

		return TRUE;
	}

	return FALSE;
}

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


static DWORD WINAPI MainThread(LPVOID hInstance)
{
	if (EntryPoint())
	{
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

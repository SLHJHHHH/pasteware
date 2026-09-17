#include <Windows.h>

#include "core/module.h"
#include "core/trace.h"

static DWORD WINAPI MainThread(LPVOID param)
{
    auto module = static_cast<HMODULE>(param);
    auto& instance = Module::instance();

    Trace::open(module);
    Trace::write("attached");

    for (int i = 0; i < 100 && !instance.start(module); ++i)
    {
        Trace::write("start retry");
        Sleep(100);
    }

    Trace::write(instance.running() ? "started" : "start timeout");

    while (instance.running())
        Sleep(100);

    FreeLibraryAndExitThread(module, 0);
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);

        auto thread = CreateThread(nullptr, 0, MainThread, module, 0, nullptr);
        if (thread)
            CloseHandle(thread);
    }

    return TRUE;
}

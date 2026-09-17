#pragma once

#include <Windows.h>

extern "C"
{
    int __stdcall MH_Initialize();
    int __stdcall MH_CreateHook(LPVOID target, LPVOID detour, LPVOID* original);
    int __stdcall MH_EnableHook(LPVOID target);
    int __stdcall MH_DisableHook(LPVOID target);
    int __stdcall MH_RemoveHook(LPVOID target);
}

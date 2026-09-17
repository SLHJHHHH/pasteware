#pragma once

#include <Windows.h>

#include "../game/types.h"

class ClientHook
{
public:
    static ClientHook& instance();

    bool install(HMODULE engine);
    void restore();

private:
    using CreateMove = void(__cdecl*)(float, usercmd_s*, int);

    static void __cdecl hook(float frametime, usercmd_s* command, int active);
    void invoke(float frametime, usercmd_s* command, int active);

    void* target_ = nullptr;
    CreateMove original_ = nullptr;
};

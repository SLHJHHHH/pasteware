#pragma once

#include <Windows.h>

class OpenGLHook
{
public:
    static OpenGLHook& instance();

    bool install(HMODULE module);
    void restore();

private:
    using SwapBuffers = BOOL(WINAPI*)(HDC);

    static BOOL WINAPI hook(HDC hdc);
    BOOL invoke(HDC hdc);

    SwapBuffers original_ = nullptr;
    void* target_ = nullptr;
};

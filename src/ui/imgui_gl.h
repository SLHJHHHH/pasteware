#pragma once

#include <Windows.h>

class ImGuiGl
{
public:
    bool initialize(HWND window);
    void shutdown();
    void new_frame();
    void render();

private:
    HWND window_ = nullptr;
    INT64 ticks_ = 0;
    INT64 time_ = 0;
    unsigned int font_ = 0;
    bool initialized_ = false;
};

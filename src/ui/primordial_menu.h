#pragma once

#include "imgui_gl.h"

class PrimordialMenu
{
public:
    static PrimordialMenu& instance();

    bool initialize();
    void shutdown();
    void render();

private:
    bool initialized_ = false;
    ImGuiGl backend_;
};

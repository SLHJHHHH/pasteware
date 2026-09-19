#pragma once
#include <string>
#include <vector>
#include <functional>
#include "includes/ImGui/imgui.h"
#include "includes/ImGui/imgui_internal.h"

struct bind_t;

namespace components
{
    const char* key_to_string(int key);

    void set_accent_color(const ImVec4& color);
    ImVec4 get_accent_color();


    bool checkbox(const char* label, bool* value, bind_t* bind = nullptr);
    bool checkbox(const char* label, int* value, bind_t* bind = nullptr);


    bool slider_int(const char* label, int* value, int min_val, int max_val, const char* format = "%d", const char* suffix = "");
    bool slider_float(const char* label, float* value, float min_val, float max_val, const char* format = "%.1f", const char* suffix = "");


    bool combo(const char* label, int* current_item, const char* const items[], int items_count);
    bool multi_combo(const char* label, bool* values, const char* const items[], int items_count);


    bool keybind(const char* label, bind_t* bind);
    bool keybind(const char* label, int* key);


    bool color_edit(const char* label, float* col_rgba);


    bool button(const char* label, const ImVec2& size = ImVec2(0.f, 0.f));
    void input_text(const char* label, char* buf, size_t buf_size);
    void text(const char* fmt, ...);
    void text_colored(const ImVec4& col, const char* fmt, ...);
    void separator();
}

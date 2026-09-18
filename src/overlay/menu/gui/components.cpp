#include "framework.h"
#include "components.h"
#include <unordered_map>
#include <cstdio>
#include <cstdarg>
#include <algorithm>

namespace components
{
    static ImVec4 g_AccentColor = ImVec4(1.0f, 0.30f, 0.62f, 1.0f);
    static std::unordered_map<ImGuiID, float> g_AnimMap;
    static ImGuiID g_WaitingKeyId = 0;
    static int g_WaitingKeyFrame = 0;

    static ImGuiID g_ActiveSliderInputId = 0;
    static bool g_SliderInputJustOpened = false;
    static int g_TypedSliderInt = 0;
    static float g_TypedSliderFloat = 0.0f;

    static float g_CopiedColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    static bool g_HasCopiedColor = false;

    void set_accent_color(const ImVec4& color)
    {
        g_AccentColor = color;
    }

    ImVec4 get_accent_color()
    {
        return g_AccentColor;
    }

    #define CHECK_KEY(vkey, gkey) if (GetAsyncKeyState(vkey) & 0x8000) return gkey;

    static int poll_pressed_key(bool ignore_mouse)
    {
        if (!ignore_mouse)
        {
            CHECK_KEY(VK_LBUTTON, K_MOUSE1);
            CHECK_KEY(VK_RBUTTON, K_MOUSE2);
            CHECK_KEY(VK_MBUTTON, K_MOUSE3);
            CHECK_KEY(VK_XBUTTON1, K_MOUSE4);
            CHECK_KEY(VK_XBUTTON2, K_MOUSE5);
        }

        // Function keys
        CHECK_KEY(VK_F1, K_F1);
        CHECK_KEY(VK_F2, K_F2);
        CHECK_KEY(VK_F3, K_F3);
        CHECK_KEY(VK_F4, K_F4);
        CHECK_KEY(VK_F5, K_F5);
        CHECK_KEY(VK_F6, K_F6);
        CHECK_KEY(VK_F7, K_F7);
        CHECK_KEY(VK_F8, K_F8);
        CHECK_KEY(VK_F9, K_F9);
        CHECK_KEY(VK_F10, K_F10);
        CHECK_KEY(VK_F11, K_F11);
        CHECK_KEY(VK_F12, K_F12);

        // Control keys
        CHECK_KEY(VK_SPACE, K_SPACE);
        CHECK_KEY(VK_RETURN, K_ENTER);
        CHECK_KEY(VK_TAB, K_TAB);
        CHECK_KEY(VK_SHIFT, K_SHIFT);
        CHECK_KEY(VK_CONTROL, K_CTRL);
        CHECK_KEY(VK_MENU, K_ALT);
        CHECK_KEY(VK_UP, K_UPARROW);
        CHECK_KEY(VK_DOWN, K_DOWNARROW);
        CHECK_KEY(VK_LEFT, K_LEFTARROW);
        CHECK_KEY(VK_RIGHT, K_RIGHTARROW);
        CHECK_KEY(VK_HOME, K_HOME);
        CHECK_KEY(VK_END, K_END);
        CHECK_KEY(VK_PRIOR, K_PGUP);
        CHECK_KEY(VK_NEXT, K_PGDN);
        CHECK_KEY(VK_INSERT, K_INS);
        CHECK_KEY(VK_DELETE, K_DEL);
        CHECK_KEY(VK_CAPITAL, K_CAPSLOCK);

        // Numbers 0-9
        for (int k = 0x30; k <= 0x39; ++k)
        {
            if (GetAsyncKeyState(k) & 0x8000)
                return K_0 + (k - 0x30);
        }

        // Letters A-Z
        for (int k = 0x41; k <= 0x5A; ++k)
        {
            if (GetAsyncKeyState(k) & 0x8000)
                return K_A + (k - 0x41);
        }

        return 0;
    }

    const char* key_to_string(int key)
    {
        if (key == 0)
            return "None";

        static std::string name_holder;
        std::string name = Game::KeynumToString(key);
        if (!name.empty())
        {
            name_holder = name;
            return name_holder.c_str();
        }

        switch (key)
        {
        case K_MOUSE1: return "M1";
        case K_MOUSE2: return "M2";
        case K_MOUSE3: return "M3";
        case K_MOUSE4: return "M4";
        case K_MOUSE5: return "M5";
        case K_SPACE: return "Space";
        case K_SHIFT: return "Shift";
        case K_CTRL: return "Ctrl";
        case K_ALT: return "Alt";
        case K_TAB: return "Tab";
        case K_ENTER: return "Enter";
        case K_ESCAPE: return "Esc";
        default:
            name_holder = std::to_string(key);
            return name_holder.c_str();
        }
    }

    bool checkbox(const char* label, bool* value, bind_t* bind)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

        const float square_sz = 14.0f;
        const ImVec2 pos = window->DC.CursorPos;
        const float total_width = ImGui::GetContentRegionAvailWidth();

        float bind_btn_w = 0.0f;
        if (bind)
            bind_btn_w = 72.0f;

        const float check_area_w = total_width - (bind ? (bind_btn_w + 8.0f) : 0.0f);
        const float bind_h = 22.0f;
        const float row_h = ImMax(square_sz + 4.0f, bind ? (bind_h + 2.0f) : (label_size.y + 4.0f));
        const ImRect total_bb(pos, pos + ImVec2(total_width, row_h));
        const ImRect check_click_bb(pos, pos + ImVec2(check_area_w, row_h));

        ImGui::ItemSize(total_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(check_click_bb, id, &hovered, &held);
        if (pressed)
        {
            *value = !(*value);
            ImGui::MarkItemEdited(id);
        }

        // Animation interpolation
        float& anim = g_AnimMap[id];
        const float target_anim = *value ? 1.0f : 0.0f;
        const float dt = ImClamp(g.IO.DeltaTime * 16.0f, 0.0f, 1.0f);
        anim = ImLerp(anim, target_anim, dt);

        const float box_y_off = (row_h - square_sz) * 0.5f;
        const ImRect check_bb(pos + ImVec2(0, box_y_off), pos + ImVec2(square_sz, box_y_off + square_sz));

        // Background color lerp: dark when off, accent when on
        const ImVec4 col_off(0.13f, 0.13f, 0.15f, 1.0f);
        const ImVec4 col_on = g_AccentColor;
        const ImVec4 current_bg = ImLerp(col_off, col_on, anim);

        const ImVec4 border_col = hovered 
            ? ImVec4(0.38f, 0.38f, 0.42f, 1.0f) 
            : ImVec4(0.20f, 0.20f, 0.23f, 1.0f);

        // Draw checkbox box
        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, ImColor(current_bg), 3.0f);
        if (anim > 0.02f)
            window->DrawList->AddRect(check_bb.Min - ImVec2(2, 2), check_bb.Max + ImVec2(2, 2), ImColor(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, 0.35f * anim), 5.0f, 15, 1.5f);
        window->DrawList->AddRect(check_bb.Min, check_bb.Max, ImColor(border_col), 3.0f);

        // Draw animated checkmark
        if (anim > 0.01f)
        {
            const float pad = 3.0f;
            const ImVec2 p1 = check_bb.Min + ImVec2(pad + 0.5f, 7.0f);
            const ImVec2 p2 = check_bb.Min + ImVec2(pad + 2.5f, 10.0f);
            const ImVec2 p3 = check_bb.Min + ImVec2(square_sz - pad, 3.5f);

            window->DrawList->PathLineTo(p1);
            window->DrawList->PathLineTo(p2);
            window->DrawList->PathLineTo(p3);
            window->DrawList->PathStroke(ImColor(1.0f, 1.0f, 1.0f, anim), false, 1.8f);
        }

        // Draw Label Text
        const ImU32 text_col = hovered 
            ? ImColor(255, 255, 255) 
            : (*value ? ImColor(230, 230, 235) : ImColor(165, 165, 175));
        window->DrawList->AddText(pos + ImVec2(square_sz + 8.f, (row_h - label_size.y) * 0.5f), text_col, label);

        // Keybind Button on right side if bind is attached
        if (bind)
        {
            const ImGuiID bind_id = window->GetID((std::string(label) + "_bind").c_str());
            const float bind_y_off = (row_h - bind_h) * 0.5f;
            const ImRect bind_bb(pos + ImVec2(total_width - bind_btn_w, bind_y_off), pos + ImVec2(total_width, bind_y_off + bind_h));

            bool bind_hovered = ImGui::IsMouseHoveringRect(bind_bb.Min, bind_bb.Max);
            if (bind_hovered && ImGui::IsMouseClicked(0))
            {
                if (g_WaitingKeyId == bind_id)
                {
                    g_WaitingKeyId = 0;
                }
                else
                {
                    g_WaitingKeyId = bind_id;
                    g_WaitingKeyFrame = ImGui::GetFrameCount();
                }
            }

            // Right click opens mode menu
            std::string popup_id = std::string("##popup_bind_") + label;
            if (bind_hovered && ImGui::IsMouseClicked(1))
            {
                ImGui::OpenPopup(popup_id.c_str());
            }

            const bool is_waiting = (g_WaitingKeyId == bind_id);
            if (is_waiting)
            {
                if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
                {
                    bind->keynum = 0;
                    g_WaitingKeyId = 0;
                }
                else
                {
                    bool ignore_mouse = (ImGui::GetFrameCount() <= g_WaitingKeyFrame + 5);
                    int pressed_key = poll_pressed_key(ignore_mouse);
                    if (pressed_key > 0)
                    {
                        bind->keynum = pressed_key;
                        g_WaitingKeyId = 0;
                        ImGui::MarkItemEdited(id);
                    }
                }
            }

            // Draw keybind button
            char bind_buf[48];
            if (is_waiting)
                sprintf_s(bind_buf, "[ ... ]");
            else
                sprintf_s(bind_buf, "[ %s ]", key_to_string(bind->keynum));

            const ImVec4 btn_bg = is_waiting 
                ? ImVec4(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, 0.35f)
                : (bind_hovered ? ImVec4(0.20f, 0.20f, 0.23f, 1.0f) : ImVec4(0.13f, 0.13f, 0.15f, 1.0f));
            
            const ImU32 btn_text_col = (bind->keynum > 0 || is_waiting) 
                ? ImColor(g_AccentColor) 
                : (bind_hovered ? ImColor(220, 220, 225) : ImColor(130, 130, 140));

            if (is_waiting)
                window->DrawList->AddRect(bind_bb.Min - ImVec2(3.f, 3.f), bind_bb.Max + ImVec2(3.f, 3.f), ImColor(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, 0.22f), 9.0f, 15, 4.0f);
            else if (bind->keynum > 0)
                window->DrawList->AddRect(bind_bb.Min - ImVec2(1.f, 1.f), bind_bb.Max + ImVec2(1.f, 1.f), ImColor(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, 0.12f), 8.0f, 15, 1.5f);
            window->DrawList->AddRectFilled(bind_bb.Min, bind_bb.Max, ImColor(btn_bg), 7.0f);
            window->DrawList->AddRect(bind_bb.Min, bind_bb.Max, is_waiting ? ImColor(g_AccentColor) : ImColor(45, 45, 52), 7.0f);

            const ImVec2 txt_sz = ImGui::CalcTextSize(bind_buf);
            const ImVec2 txt_pos(bind_bb.Min.x + (bind_btn_w - txt_sz.x) * 0.5f, bind_bb.Min.y + (bind_h - txt_sz.y) * 0.5f);
            window->DrawList->AddText(txt_pos, btn_text_col, bind_buf);

            // Bind Options Popup (Hold / Toggle / Clear)
            if (ImGui::BeginPopup(popup_id.c_str()))
            {
                ImGui::TextDisabled("Activation Mode");
                ImGui::Separator();
                if (ImGui::MenuItem("Hold (On Press)", NULL, bind->type == 1))
                    bind->type = 1;
                if (ImGui::MenuItem("Toggle", NULL, bind->type == 2))
                    bind->type = 2;
                ImGui::Separator();
                if (ImGui::MenuItem("Clear Key"))
                    bind->keynum = 0;
                ImGui::EndPopup();
            }
        }

        return pressed;
    }

    bool checkbox(const char* label, int* value, bind_t* bind)
    {
        bool b = (*value != 0);
        if (checkbox(label, &b, bind))
        {
            *value = b ? 1 : 0;
            return true;
        }
        return false;
    }

    bool slider_int(const char* label, int* value, int min_val, int max_val, const char* format, const char* suffix)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        const float total_width = ImGui::GetContentRegionAvailWidth();
        const ImVec2 pos = window->DC.CursorPos;

        // Draw Label
        ImGui::TextUnformatted(label);

        // Draw Value string on right (clickable for manual text editing)
        char val_buf[64];
        if (suffix && suffix[0] != '\0')
            sprintf_s(val_buf, "%d %s", *value, suffix);
        else
            sprintf_s(val_buf, format, *value);

        const ImVec2 val_size = ImGui::CalcTextSize(val_buf);
        const float val_click_w = ImMax(val_size.x + 8.0f, 44.0f);
        const ImRect val_bb(pos + ImVec2(total_width - val_click_w, 0.0f), pos + ImVec2(total_width, ImGui::GetTextLineHeight() + 2.0f));

        const bool is_editing = (g_ActiveSliderInputId == id);
        if (!is_editing)
        {
            bool val_hovered = ImGui::IsMouseHoveringRect(val_bb.Min, val_bb.Max);
            if (val_hovered && ImGui::IsMouseClicked(0))
            {
                g_ActiveSliderInputId = id;
                g_SliderInputJustOpened = true;
                g_TypedSliderInt = *value;
            }

            ImGui::SameLine(total_width - val_size.x);
            ImGui::PushStyleColor(ImGuiCol_Text, val_hovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.70f, 0.70f, 0.75f, 1.0f));
            ImGui::TextUnformatted(val_buf);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::SameLine(total_width - 55.0f);
            ImGui::PushItemWidth(55.0f);
            std::string input_id = std::string("##val_input_") + label;
            if (g_SliderInputJustOpened)
            {
                ImGui::SetKeyboardFocusHere();
                g_SliderInputJustOpened = false;
            }
            if (ImGui::InputInt(input_id.c_str(), &g_TypedSliderInt, 0, 0, ImGuiInputTextFlags_EnterReturnsTrue))
            {
                *value = std::clamp(g_TypedSliderInt, min_val, max_val);
                g_ActiveSliderInputId = 0;
                ImGui::MarkItemEdited(id);
            }
            else if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered())
            {
                *value = std::clamp(g_TypedSliderInt, min_val, max_val);
                g_ActiveSliderInputId = 0;
                ImGui::MarkItemEdited(id);
            }
            ImGui::PopItemWidth();
        }

        // Custom slim slider bar
        const float track_h = 4.0f;
        const float pad_y = 6.0f;
        const ImRect track_bb(pos + ImVec2(0, ImGui::GetTextLineHeight() + pad_y), pos + ImVec2(total_width, ImGui::GetTextLineHeight() + pad_y + track_h));
        const ImRect click_bb(track_bb.Min - ImVec2(0, 4.0f), track_bb.Max + ImVec2(0, 4.0f));

        ImGui::ItemSize(ImRect(pos, pos + ImVec2(total_width, ImGui::GetTextLineHeight() + track_h + pad_y + 4.0f)), style.FramePadding.y);
        if (!ImGui::ItemAdd(click_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(click_bb, id, &hovered, &held);

        if (held)
        {
            const float mouse_norm = ImClamp((g.IO.MousePos.x - track_bb.Min.x) / track_bb.GetWidth(), 0.0f, 1.0f);
            const int new_val = static_cast<int>(min_val + mouse_norm * (max_val - min_val) + 0.5f);
            if (*value != new_val)
            {
                *value = new_val;
                ImGui::MarkItemEdited(id);
            }
        }

        // Calculate grab ratio
        const float ratio = ImClamp(static_cast<float>(*value - min_val) / static_cast<float>(max_val - min_val), 0.0f, 1.0f);
        const float grab_x = track_bb.Min.x + ratio * track_bb.GetWidth();

        // Track background
        window->DrawList->AddRectFilled(track_bb.Min, track_bb.Max, ImColor(30, 30, 36), track_h * 0.5f);
        
        // Filled portion
        if (ratio > 0.0f)
        {
            window->DrawList->AddRectFilled(track_bb.Min, ImVec2(grab_x, track_bb.Max.y), ImColor(g_AccentColor), track_h * 0.5f);
        }

        // Grabber circle
        const float grab_radius = (hovered || held) ? 5.5f : 4.5f;
        window->DrawList->AddCircleFilled(ImVec2(grab_x, track_bb.Min.y + track_h * 0.5f), grab_radius, ImColor(255, 255, 255));
        window->DrawList->AddCircle(ImVec2(grab_x, track_bb.Min.y + track_h * 0.5f), grab_radius, ImColor(g_AccentColor), 12, 1.2f);

        ImGui::Spacing();
        return held;
    }

    bool slider_float(const char* label, float* value, float min_val, float max_val, const char* format, const char* suffix)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        const float total_width = ImGui::GetContentRegionAvailWidth();
        const ImVec2 pos = window->DC.CursorPos;

        // Draw Label
        ImGui::TextUnformatted(label);

        // Draw Value string on right (clickable for manual text editing)
        char val_buf[64];
        if (suffix && suffix[0] != '\0')
            sprintf_s(val_buf, "%.1f %s", *value, suffix);
        else
            sprintf_s(val_buf, format, *value);

        const ImVec2 val_size = ImGui::CalcTextSize(val_buf);
        const float val_click_w = ImMax(val_size.x + 8.0f, 44.0f);
        const ImRect val_bb(pos + ImVec2(total_width - val_click_w, 0.0f), pos + ImVec2(total_width, ImGui::GetTextLineHeight() + 2.0f));

        const bool is_editing = (g_ActiveSliderInputId == id);
        if (!is_editing)
        {
            bool val_hovered = ImGui::IsMouseHoveringRect(val_bb.Min, val_bb.Max);
            if (val_hovered && ImGui::IsMouseClicked(0))
            {
                g_ActiveSliderInputId = id;
                g_SliderInputJustOpened = true;
                g_TypedSliderFloat = *value;
            }

            ImGui::SameLine(total_width - val_size.x);
            ImGui::PushStyleColor(ImGuiCol_Text, val_hovered ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.70f, 0.70f, 0.75f, 1.0f));
            ImGui::TextUnformatted(val_buf);
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::SameLine(total_width - 55.0f);
            ImGui::PushItemWidth(55.0f);
            std::string input_id = std::string("##val_f_input_") + label;
            if (g_SliderInputJustOpened)
            {
                ImGui::SetKeyboardFocusHere();
                g_SliderInputJustOpened = false;
            }
            if (ImGui::InputFloat(input_id.c_str(), &g_TypedSliderFloat, 0.0f, 0.0f, "%.1f", ImGuiInputTextFlags_EnterReturnsTrue))
            {
                *value = std::clamp(g_TypedSliderFloat, min_val, max_val);
                g_ActiveSliderInputId = 0;
                ImGui::MarkItemEdited(id);
            }
            else if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0) && !ImGui::IsItemHovered())
            {
                *value = std::clamp(g_TypedSliderFloat, min_val, max_val);
                g_ActiveSliderInputId = 0;
                ImGui::MarkItemEdited(id);
            }
            ImGui::PopItemWidth();
        }

        // Custom slim slider bar
        const float track_h = 4.0f;
        const float pad_y = 6.0f;
        const ImRect track_bb(pos + ImVec2(0, ImGui::GetTextLineHeight() + pad_y), pos + ImVec2(total_width, ImGui::GetTextLineHeight() + pad_y + track_h));
        const ImRect click_bb(track_bb.Min - ImVec2(0, 4.0f), track_bb.Max + ImVec2(0, 4.0f));

        ImGui::ItemSize(ImRect(pos, pos + ImVec2(total_width, ImGui::GetTextLineHeight() + track_h + pad_y + 4.0f)), style.FramePadding.y);
        if (!ImGui::ItemAdd(click_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(click_bb, id, &hovered, &held);

        if (held)
        {
            const float mouse_norm = ImClamp((g.IO.MousePos.x - track_bb.Min.x) / track_bb.GetWidth(), 0.0f, 1.0f);
            const float new_val = min_val + mouse_norm * (max_val - min_val);
            if (*value != new_val)
            {
                *value = new_val;
                ImGui::MarkItemEdited(id);
            }
        }

        // Calculate grab ratio
        const float ratio = ImClamp((*value - min_val) / (max_val - min_val), 0.0f, 1.0f);
        const float grab_x = track_bb.Min.x + ratio * track_bb.GetWidth();

        // Track background
        window->DrawList->AddRectFilled(track_bb.Min, track_bb.Max, ImColor(30, 30, 36), track_h * 0.5f);
        
        // Filled portion
        if (ratio > 0.0f)
        {
            window->DrawList->AddRectFilled(track_bb.Min, ImVec2(grab_x, track_bb.Max.y), ImColor(g_AccentColor), track_h * 0.5f);
        }

        // Grabber circle
        const float grab_radius = (hovered || held) ? 5.5f : 4.5f;
        window->DrawList->AddCircleFilled(ImVec2(grab_x, track_bb.Min.y + track_h * 0.5f), grab_radius, ImColor(255, 255, 255));
        window->DrawList->AddCircle(ImVec2(grab_x, track_bb.Min.y + track_h * 0.5f), grab_radius, ImColor(g_AccentColor), 12, 1.2f);

        ImGui::Spacing();
        return held;
    }

    bool combo(const char* label, int* current_item, const char* const items[], int items_count)
    {
        ImGui::TextUnformatted(label);
        ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
        std::string id = std::string("##combo_") + label;
        bool changed = ImGui::Combo(id.c_str(), current_item, items, items_count);
        ImGui::PopItemWidth();
        ImGui::Spacing();
        return changed;
    }

    bool multi_combo(const char* label, bool* values, const char* const items[], int items_count)
    {
        ImGui::TextUnformatted(label);
        
        std::string preview = "";
        int selected_count = 0;
        for (int i = 0; i < items_count; ++i)
        {
            if (values[i])
            {
                if (selected_count > 0)
                    preview += ", ";
                preview += items[i];
                selected_count++;
            }
        }
        if (selected_count == 0)
            preview = "None";

        ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
        std::string combo_id = std::string("##multi_") + label;
        bool changed = false;
        if (ImGui::BeginCombo(combo_id.c_str(), preview.c_str()))
        {
            for (int i = 0; i < items_count; ++i)
            {
                if (ImGui::Selectable(items[i], values[i], ImGuiSelectableFlags_DontClosePopups))
                {
                    values[i] = !values[i];
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::Spacing();
        return changed;
    }

    bool keybind(const char* label, bind_t* bind)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const ImGuiID id = window->GetID(label);
        const float total_width = ImGui::GetContentRegionAvailWidth();
        const float btn_w = 80.0f;

        ImGui::TextUnformatted(label);

        const bool is_waiting = (g_WaitingKeyId == id);
        if (is_waiting)
        {
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
            {
                bind->keynum = 0;
                g_WaitingKeyId = 0;
            }
            else
            {
                bool ignore_mouse = (ImGui::GetFrameCount() <= g_WaitingKeyFrame + 5);
                int pressed_key = poll_pressed_key(ignore_mouse);
                if (pressed_key > 0)
                {
                    bind->keynum = pressed_key;
                    g_WaitingKeyId = 0;
                    ImGui::MarkItemEdited(id);
                }
            }
        }

        char btn_buf[64];
        if (is_waiting)
            sprintf_s(btn_buf, "[ ... ]");
        else
            sprintf_s(btn_buf, "[ %s ]", key_to_string(bind->keynum));

        ImGui::SameLine(total_width - btn_w);
        const ImVec4 btn_col = is_waiting 
            ? ImVec4(g_AccentColor.x, g_AccentColor.y, g_AccentColor.z, 0.4f) 
            : ((bind->keynum > 0) ? ImVec4(0.18f, 0.18f, 0.22f, 1.0f) : ImVec4(0.13f, 0.13f, 0.15f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, btn_col);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.26f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.26f, 0.26f, 0.30f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, (bind->keynum > 0 || is_waiting) ? g_AccentColor : ImVec4(0.65f, 0.65f, 0.70f, 1.0f));
        
        bool pressed = ImGui::Button(btn_buf, ImVec2(btn_w, 24.0f));
        ImGui::PopStyleColor(4);

        if (pressed)
        {
            if (g_WaitingKeyId == id)
            {
                g_WaitingKeyId = 0;
            }
            else
            {
                g_WaitingKeyId = id;
                g_WaitingKeyFrame = ImGui::GetFrameCount();
            }
        }

        // Right-click opens popup for Hold / Toggle / Clear
        std::string popup_id = std::string("##popup_mode_") + label;
        if (ImGui::IsItemClicked(1))
        {
            ImGui::OpenPopup(popup_id.c_str());
        }

        if (ImGui::BeginPopup(popup_id.c_str()))
        {
            ImGui::TextDisabled("Activation Mode");
            ImGui::Separator();
            if (ImGui::MenuItem("Hold (On Press)", NULL, bind->type == 1))
                bind->type = 1;
            if (ImGui::MenuItem("Toggle", NULL, bind->type == 2))
                bind->type = 2;
            ImGui::Separator();
            if (ImGui::MenuItem("Clear Key"))
                bind->keynum = 0;
            ImGui::EndPopup();
        }

        ImGui::Spacing();
        return pressed;
    }

    bool keybind(const char* label, int* key)
    {
        bind_t temp_bind{ *key, 1 };
        if (keybind(label, &temp_bind))
        {
            *key = temp_bind.keynum;
            return true;
        }
        *key = temp_bind.keynum;
        return false;
    }

    bool color_edit(const char* label, float* col_rgba)
    {
        ImGui::TextUnformatted(label);
        ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - 28.0f);
        std::string id = std::string("##col_") + label;
        std::string popup_id = std::string("##col_popup_") + label;

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImRect col_bb(pos, pos + ImVec2(24.0f, 14.0f));

        bool hovered = ImGui::IsMouseHoveringRect(col_bb.Min, col_bb.Max);
        if (hovered && ImGui::IsMouseClicked(1))
        {
            ImGui::OpenPopup(popup_id.c_str());
        }

        bool changed = ImGui::ColorEdit4(id.c_str(), col_rgba, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);

        if (ImGui::BeginPopup(popup_id.c_str()))
        {
            ImGui::TextDisabled("Color Options");
            ImGui::Separator();
            if (ImGui::MenuItem("Copy Color"))
            {
                g_CopiedColor[0] = col_rgba[0];
                g_CopiedColor[1] = col_rgba[1];
                g_CopiedColor[2] = col_rgba[2];
                g_CopiedColor[3] = col_rgba[3];
                g_HasCopiedColor = true;

                char hex_buf[32];
                sprintf_s(hex_buf, "#%02X%02X%02X%02X",
                    static_cast<int>(col_rgba[0] * 255.f),
                    static_cast<int>(col_rgba[1] * 255.f),
                    static_cast<int>(col_rgba[2] * 255.f),
                    static_cast<int>(col_rgba[3] * 255.f));
                ImGui::SetClipboardText(hex_buf);
            }
            if (ImGui::MenuItem("Paste Color", NULL, false, g_HasCopiedColor))
            {
                col_rgba[0] = g_CopiedColor[0];
                col_rgba[1] = g_CopiedColor[1];
                col_rgba[2] = g_CopiedColor[2];
                col_rgba[3] = g_CopiedColor[3];
                changed = true;
            }
            ImGui::EndPopup();
        }

        ImGui::Spacing();
        return changed;
    }

    bool button(const char* label, const ImVec2& size)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.18f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.22f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.28f, 0.28f, 0.32f, 1.0f));
        bool pressed = ImGui::Button(label, size);
        ImGui::PopStyleColor(3);
        return pressed;
    }

    void text(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        ImGui::TextV(fmt, args);
        va_end(args);
    }

    void text_colored(const ImVec4& col, const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::TextV(fmt, args);
        ImGui::PopStyleColor();
        va_end(args);
    }

    void separator()
    {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }
}

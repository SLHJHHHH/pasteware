#include "primordial_menu.h"

#include "menu.h"
#include "../config/config.h"

#include <Windows.h>
#include <imgui.h>

ImFont* LexendDecaFont = nullptr;
ImFont* InterMedium = nullptr;
ImFont* IconFontLogs = nullptr;

namespace
{
    HWND find_window()
    {
        HWND result = nullptr;
        EnumWindows([](HWND window, LPARAM data)
        {
            DWORD process = 0;
            GetWindowThreadProcessId(window, &process);
            if (process != GetCurrentProcessId() || !IsWindowVisible(window) || GetWindow(window, GW_OWNER))
                return TRUE;

            *reinterpret_cast<HWND*>(data) = window;
            return FALSE;
        }, reinterpret_cast<LPARAM>(&result));
        return result;
    }

    void tab(const char* name, int index)
    {
        auto& menu = Menu::instance();
        const bool active = menu.selected() == static_cast<std::size_t>(index);
        if (active)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.76f, 0.60f, 0.64f, 1.0f));

        if (ImGui::Button(name, ImVec2(108.0f, 34.0f)))
            menu.select(index);

        if (active)
            ImGui::PopStyleColor();
    }
}

PrimordialMenu& PrimordialMenu::instance()
{
    static PrimordialMenu menu;
    return menu;
}

bool PrimordialMenu::initialize()
{
    if (initialized_)
        return true;

    auto window = find_window();
    if (!window)
        return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    auto& io = ImGui::GetIO();
    LexendDecaFont = io.Fonts->AddFontDefault();
    InterMedium = io.Fonts->AddFontDefault();
    IconFontLogs = io.Fonts->AddFontDefault();
    if (!backend_.initialize(window))
    {
        ImGui::DestroyContext();
        return false;
    }

    initialized_ = true;
    return true;
}

void PrimordialMenu::shutdown()
{
    if (!initialized_)
        return;

    backend_.shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

void PrimordialMenu::render()
{
    if (!Menu::instance().open())
        return;

    if (!initialize())
        return;

    backend_.new_frame();
    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(876.0f, 623.0f), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(120.0f, 90.0f), ImGuiCond_Once);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("##pasteware", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBringToFrontOnFocus);

    const ImVec2 pos = ImGui::GetWindowPos();
    auto* draw = ImGui::GetWindowDrawList();
    const ImU32 bg = IM_COL32(29, 29, 29, 255);
    const ImU32 panel = IM_COL32(41, 41, 41, 255);
    const ImU32 side = IM_COL32(34, 34, 34, 255);
    const ImU32 accent = IM_COL32(193, 154, 164, 255);
    const auto at = [&pos](float x, float y) { return ImVec2(pos.x + x, pos.y + y); };
    draw->AddRectFilled(pos, at(863.0f, 610.0f), bg, 10.0f);
    draw->AddRectFilled(pos, at(863.0f, 55.0f), panel, 10.0f, ImDrawCornerFlags_Top);
    draw->AddRectFilled(at(0.0f, 540.0f), at(863.0f, 610.0f), panel, 10.0f, ImDrawCornerFlags_Bot);
    draw->AddRectFilled(at(0.0f, 56.0f), at(144.0f, 539.0f), side);
    draw->AddLine(at(0.0f, 55.0f), at(863.0f, 55.0f), accent);
    draw->AddLine(at(0.0f, 539.0f), at(863.0f, 539.0f), accent);
    draw->AddRect(pos, at(863.0f, 610.0f), IM_COL32(41, 41, 41, 255), 10.0f);
    draw->AddText(at(18.0f, 18.0f), IM_COL32(230, 230, 230, 255), "pasteware");
    draw->AddText(at(110.0f, 18.0f), IM_COL32(127, 127, 127, 255), "community build");

    ImGui::SetCursorPos(ImVec2(230.0f, 565.0f));
    tab("Aimbot", 0); ImGui::SameLine();
    tab("Antiaim", 1); ImGui::SameLine();
    tab("Visuals", 2); ImGui::SameLine();
    tab("Movement", 3); ImGui::SameLine();
    tab("Config", 5);

    ImGui::SetCursorPos(ImVec2(166.0f, 88.0f));
    ImGui::BeginChild("content", ImVec2(660.0f, 420.0f), false);
    auto& config = Config::instance();
    switch (Menu::instance().selected())
    {
    case 0:
        {
            bool enabled = config.get_bool("aim.enabled");
            bool silent = config.get_bool("aim.silent", false);
            bool team = config.get_bool("aim.team", false);
            float fov = config.get_float("aim.fov", 12.0f);
            float smooth = config.get_float("aim.smooth", 1.0f);
            ImGui::TextUnformatted("Aimbot");
            ImGui::Checkbox("Enabled", &enabled);
            ImGui::Checkbox("Silent", &silent);
            ImGui::Checkbox("Target teammates", &team);
            ImGui::SliderFloat("FOV", &fov, 1.0f, 45.0f, "%.1f");
            ImGui::SliderFloat("Smooth", &smooth, 1.0f, 20.0f, "%.1f");
            config.set_bool("aim.enabled", enabled);
            config.set_bool("aim.silent", silent);
            config.set_bool("aim.team", team);
            config.set_float("aim.fov", fov);
            config.set_float("aim.smooth", smooth);
        }
        break;
    case 1:
        {
            bool enabled = config.get_bool("antiaim.enabled");
            float pitch = config.get_float("antiaim.pitch", 89.0f);
            float yaw = config.get_float("antiaim.yaw", 180.0f);
            bool jitter = config.get_bool("antiaim.jitter", false);
            ImGui::Checkbox("Enabled", &enabled);
            ImGui::Checkbox("Jitter", &jitter);
            ImGui::SliderFloat("Pitch", &pitch, -89.0f, 89.0f, "%.1f");
            ImGui::SliderFloat("Yaw", &yaw, -180.0f, 180.0f, "%.1f");
            config.set_bool("antiaim.enabled", enabled);
            config.set_bool("antiaim.jitter", jitter);
            config.set_float("antiaim.pitch", pitch);
            config.set_float("antiaim.yaw", yaw);
        }
        break;
    case 2:
        {
            bool esp = config.get_bool("visuals.esp", true);
            bool crosshair = config.get_bool("visuals.crosshair", true);
            bool box = config.get_bool("visuals.box", true);
            bool names = config.get_bool("visuals.names", false);
            ImGui::Checkbox("Player ESP", &esp);
            ImGui::Checkbox("Crosshair", &crosshair);
            ImGui::Checkbox("Boxes", &box);
            ImGui::Checkbox("Names", &names);
            config.set_bool("visuals.esp", esp);
            config.set_bool("visuals.crosshair", crosshair);
            config.set_bool("visuals.box", box);
            config.set_bool("visuals.names", names);
        }
        break;
    case 3:
        {
            bool bhop = config.get_bool("movement.bhop", true);
            bool autostrafe = config.get_bool("movement.autostrafe", false);
            ImGui::TextUnformatted("Movement");
            ImGui::Checkbox("Bunnyhop", &bhop);
            ImGui::Checkbox("Autostrafe", &autostrafe);
            config.set_bool("movement.bhop", bhop);
            config.set_bool("movement.autostrafe", autostrafe);
        }
        break;
    case 5:
        if (ImGui::Button("Save"))
            config.save("pasteware.cfg");
        ImGui::SameLine();
        if (ImGui::Button("Load"))
            config.load("pasteware.cfg");
        break;
    }

    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();
    backend_.render();
}

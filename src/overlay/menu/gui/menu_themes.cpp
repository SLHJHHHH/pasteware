#include "framework.h"

void CMenuThemes::SetDefaultDarkTheme()
{
	auto& style = GImGui->Style;
	style.WindowRounding = 12.0f;
	style.ChildRounding = 8.0f;
	style.FrameRounding = 6.0f;
	style.GrabRounding = 10.0f;
	style.PopupRounding = 8.0f;
	style.ScrollbarRounding = 12.0f;
	style.ScrollbarSize = 7.0f;
	style.WindowBorderSize = 1.0f;
	style.ChildBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(10.0f, 7.0f);
	style.WindowPadding = ImVec2(14.0f, 12.0f);
	style.FramePadding = ImVec2(8.0f, 4.0f);
	auto* colors = style.Colors;
	colors[ImGuiCol_WindowBg] = ImVec4(0.055f, 0.055f, 0.085f, 0.99f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.095f, 0.095f, 0.135f, 0.98f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.99f);
	colors[ImGuiCol_Border] = ImVec4(0.32f, 0.30f, 0.46f, 0.42f);
	colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 0.18f, 0.55f, 0.10f);
	colors[ImGuiCol_Text] = ImVec4(0.93f, 0.93f, 0.97f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.42f, 0.42f, 0.52f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(1.00f, 0.18f, 0.55f, 0.30f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 0.18f, 0.55f, 0.48f);
	colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 0.22f, 0.60f, 0.72f);
	colors[ImGuiCol_Button] = ImVec4(0.13f, 0.13f, 0.19f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 0.18f, 0.55f, 0.48f);
	colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.22f, 0.60f, 0.88f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.13f, 0.13f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.27f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.20f, 0.32f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.25f, 0.62f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(1.00f, 0.25f, 0.62f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.40f, 0.72f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.06f, 0.06f, 0.09f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.06f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.06f, 0.09f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.08f, 0.00f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.00f, 0.25f, 0.62f, 0.32f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.00f, 0.25f, 0.62f, 0.60f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 0.32f, 0.66f, 0.85f);
	colors[ImGuiCol_Separator] = ImVec4(1.00f, 0.25f, 0.62f, 0.20f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(1.00f, 0.25f, 0.62f, 0.42f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.25f, 0.62f, 0.62f);
}

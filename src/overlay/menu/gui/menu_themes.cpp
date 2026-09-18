#include "framework.h"

void CMenuThemes::SetDefaultDarkTheme()
{
	auto& style = GImGui->Style;
	style.WindowRounding = 10.0f;
	style.ChildRounding = 8.0f;
	style.FrameRounding = 5.0f;
	style.GrabRounding = 5.0f;
	style.PopupRounding = 8.0f;
	style.ScrollbarRounding = 4.0f;
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(10.0f, 8.0f);
	style.WindowPadding = ImVec2(14.0f, 14.0f);

	auto* colors = style.Colors;
	colors[ImGuiCol_WindowBg]             = ImVec4(0.08f, 0.08f, 0.11f, 0.98f);
	colors[ImGuiCol_ChildBg]              = ImVec4(0.11f, 0.11f, 0.15f, 0.90f);
	colors[ImGuiCol_PopupBg]              = ImVec4(0.10f, 0.10f, 0.14f, 0.98f);
	colors[ImGuiCol_Border]               = ImVec4(0.20f, 0.20f, 0.28f, 0.40f);
	colors[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	colors[ImGuiCol_Text]                 = ImVec4(0.92f, 0.92f, 0.96f, 1.00f);
	colors[ImGuiCol_TextDisabled]         = ImVec4(0.45f, 0.45f, 0.52f, 1.00f);
	colors[ImGuiCol_Header]               = ImVec4(0.95f, 0.20f, 0.55f, 0.25f);
	colors[ImGuiCol_HeaderHovered]        = ImVec4(0.95f, 0.20f, 0.55f, 0.40f);
	colors[ImGuiCol_HeaderActive]         = ImVec4(0.95f, 0.20f, 0.55f, 0.65f);

	colors[ImGuiCol_Button]               = ImVec4(0.14f, 0.14f, 0.20f, 1.00f);
	colors[ImGuiCol_ButtonHovered]        = ImVec4(0.85f, 0.15f, 0.45f, 0.50f);
	colors[ImGuiCol_ButtonActive]         = ImVec4(0.95f, 0.20f, 0.55f, 0.80f);

	colors[ImGuiCol_FrameBg]              = ImVec4(0.14f, 0.14f, 0.20f, 1.00f);
	colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.18f, 0.18f, 0.26f, 1.00f);
	colors[ImGuiCol_FrameBgActive]        = ImVec4(0.22f, 0.22f, 0.32f, 1.00f);

	colors[ImGuiCol_CheckMark]            = ImVec4(0.95f, 0.20f, 0.55f, 1.00f);
	colors[ImGuiCol_SliderGrab]           = ImVec4(0.85f, 0.15f, 0.45f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.95f, 0.20f, 0.55f, 1.00f);

	colors[ImGuiCol_TitleBg]              = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	colors[ImGuiCol_TitleBgActive]        = ImVec4(0.09f, 0.07f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
}
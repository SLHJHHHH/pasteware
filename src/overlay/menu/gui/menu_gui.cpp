#include "menu_gui.h"
#include "framework.h"
#include "overlay/font_atlas.h"
#include "components.h"

CMenuGui::CMenuGui() 
	: m_pPopupModal(std::make_unique<CMenuPopupModal>())
{
	memset(m_search, 0, sizeof(m_search));
}

bool CMenuGui::Begin(std::string name, const ImVec2& size)
{
	ImVec2 size_const = size;

	size_const.x = min(size_const.x, GImGui->IO.DisplaySize.x);
	size_const.y = min(size_const.y, GImGui->IO.DisplaySize.y);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());

	ImGui::SetNextWindowSizeConstraints(size_const, GImGui->IO.DisplaySize);
	ImGui::SetNextWindowSize(size, ImGuiCond_Once);
	ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	if (!cvars::visuals.main_allow_resize)
		window_flags |= ImGuiWindowFlags_NoResize;

	const bool ret = ImGui::Begin(name.c_str(), nullptr, window_flags);

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	return ret;
}

void CMenuGui::BeginCanvas(std::string name, const ImVec2& size, bool border)
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, GImGui->Style.Colors[ImGuiCol_WindowBg]);
	ImGui::BeginChild(name.c_str(), size, border);
	ImGui::PopStyleColor();
}

void CMenuGui::BeginGroupBox(std::string name, const ImVec2& size, bool border)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());
	ImGui::BeginChildCustom(name.c_str(), size, border);
	ImGui::PopStyleVar();
}

void CMenuGui::BeginWidgetsGroup()
{
	ImGui::Spacing(WINDOW_WIDGETS_PADDING_SPACING);
	ImGui::SameLine(WINDOW_WIDGETS_PADDING_SAMELINE);
	ImGui::BeginGroup();
}

void CMenuGui::End()
{
	ImVec2 window_pos = GImGui->CurrentWindow->Pos;
	ImVec2 window_size = GImGui->CurrentWindow->Size;

	GImGui->CurrentWindow->DrawList->PushClipRectFullScreen();

	RectangleShadowSettings shadowSettings;
	shadowSettings.shadowColor = GImGui->Style.Colors[ImGuiCol_BorderShadow];
	ImGui::DrawRectangleShadowVerticesAdaptive(shadowSettings, window_pos, window_size);

	ImGui::End();
}

void CMenuGui::EndCanvas()
{
	ImGui::EndChild();
}

void CMenuGui::EndGroupBox()
{
	ImGui::EndChildCustom();
}

void CMenuGui::EndWidgetsGroup()
{
	ImGui::Spacing(3);
	ImGui::EndGroup();
}

void CMenuGui::SmallGroupBox(std::string name, const ImVec2& size, std::function<void()> widget_first, std::function<void()> widget_second)
{
	BeginGroupBox(name, size);

	std::string widgets_names[2] = { name + "widget_first", name + "widget_second" };

	ImGui::BeginChild(widgets_names[0].c_str(), ImVec2(size.x * 0.5f, size.y));
	ImGui::SetCursorPos(ImVec2(WINDOW_WIDGETS_PADDING_SAMELINE, 9));
	widget_first();
	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild(widgets_names[1].c_str(), ImVec2(size.x - ImGui::GetCursorPosX(), size.y));
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + WINDOW_WIDGETS_PADDING_SAMELINE, 9));
	widget_second();
	ImGui::EndChild();

	EndGroupBox();
}

void CMenuGui::UnderBackground()
{
}

void CMenuGui::UnderContents()
{
	ImGui::PushFont(g_pFontList[Verdana_13px]);

	ImGui::PopFont();
}

void CMenuGui::TabBackground()
{
	auto p = GImGui->CurrentWindow->Pos;
	auto s = GImGui->CurrentWindow->Size;
	auto d = GImGui->CurrentWindow->DrawList;
	ImVec4 accent = components::get_accent_color();

	// Main background and panels
	d->AddRectFilled(p, p + s, ImColor(27, 27, 29), 9.f);
	d->AddRectFilled(p, p + ImVec2(s.x, 56.f), ImColor(39, 39, 41), 9.f, ImDrawCornerFlags_Top);
	d->AddRectFilled(p + ImVec2(0.f, s.y - 71.f), p + s, ImColor(39, 39, 41), 9.f, ImDrawCornerFlags_Bot);
	d->AddRectFilled(p + ImVec2(0.f, 56.f), p + ImVec2(145.f, s.y - 71.f), ImColor(31, 31, 33));
	d->AddLine(p + ImVec2(145.f, 56.f), p + ImVec2(145.f, s.y - 71.f), ImColor(47, 47, 49));
	d->AddRect(p, p + s, ImColor(49, 49, 51), 9.f);

	// Theme-driven separating lines with subtle accent glow
	d->AddLine(p + ImVec2(0.f, 55.f), p + ImVec2(s.x, 55.f), ImColor(accent));
	d->AddLine(p + ImVec2(0.f, s.y - 71.f), p + ImVec2(s.x, s.y - 71.f), ImColor(accent));

	// Animated shimmering text for "pasteware" title
	const char* title = "pasteware";
	const float time = static_cast<float>(ImGui::GetTime());
	float cur_x = p.x + 24.f;
	const float font_size = 19.f;

	for (int ci = 0; title[ci] != '\0'; ++ci)
	{
		char ch[2] = { title[ci], '\0' };
		const float wave = sinf(time * 3.5f + static_cast<float>(ci) * 0.45f) * 0.5f + 0.5f;
		const ImVec4 letter_col = ImLerp(accent, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), wave);

		if (g_pPrimTextFont)
			d->AddText(g_pPrimTextFont, font_size, ImVec2(cur_x, p.y + 18.f), ImColor(letter_col), ch);
		else
			d->AddText(ImVec2(cur_x, p.y + 18.f), ImColor(letter_col), ch);

		const ImVec2 ch_size = g_pPrimTextFont ? g_pPrimTextFont->CalcTextSizeA(font_size, FLT_MAX, 0.f, ch) : ImGui::CalcTextSize(ch);
		cur_x += ch_size.x;
	}

	ImGui::SetCursorPos(ImVec2(s.x - 188.f, 15.f));
	ImGui::PushItemWidth(150.f);
	ImGui::InputTextWithHint("##search", "Search...", m_search, sizeof(m_search));
	ImGui::PopItemWidth();
}

const char* CMenuGui::SearchText()
{
	return m_search;
}

bool CMenuGui::TabList(std::vector<TabWidgetsData>& data, int& selected, int& hovered)
{
	bool changed = false;
	const ImVec2 button_size(72.f, 50.f);
	const float gap = 7.f;
	const float width = data.size() * button_size.x + (data.size() - 1) * gap;
	const float x = (GImGui->CurrentWindow->Size.x - width) * 0.5f;
	const float y = GImGui->CurrentWindow->Size.y - 61.f;
	static const char icon_chars[] = { 'A', 'B', 'E', 'C', 'F', 'D', 'G' };

	ImGui::SetCursorPos(ImVec2(x, y));

	const ImVec4 accent = components::get_accent_color();
	const ImVec2 win_pos = GImGui->CurrentWindow->Pos;
	ImDrawList* draw = GImGui->CurrentWindow->DrawList;

	// The highlight slides to the selected tab instead of jumping; snapped on the first frame.
	static float slide_x = -1.f;
	const float target_x = win_pos.x + x + selected * (button_size.x + gap) + button_size.x * 0.5f;
	if (slide_x < 0.f || selected < 0 || selected >= static_cast<int>(data.size()))
		slide_x = target_x;
	else
		slide_x += (target_x - slide_x) * (1.f - expf(-18.f * GImGui->IO.DeltaTime));

	if (selected >= 0 && selected < static_cast<int>(data.size()))
	{
		// Sliding plate: tinted rounded body, accent outline, a glow rising from the bottom edge and a short accent bar.
		const ImVec2 plate_min(slide_x - button_size.x * 0.5f, win_pos.y + y);
		const ImVec2 plate_max(slide_x + button_size.x * 0.5f, win_pos.y + y + button_size.y);
		const float rounding = 10.f;

		draw->AddRectFilled(plate_min, plate_max, ImColor(accent.x, accent.y, accent.z, 0.13f), rounding);
		// Gradients cannot be rounded, so the glow is inset enough to stay inside the corner arcs (r = 10).
		draw->AddRectFilledMultiColor(ImVec2(plate_min.x + 6.f, plate_max.y - 22.f), ImVec2(plate_max.x - 6.f, plate_max.y - 1.5f),
			ImColor(accent.x, accent.y, accent.z, 0.f), ImColor(accent.x, accent.y, accent.z, 0.f),
			ImColor(accent.x, accent.y, accent.z, 0.20f), ImColor(accent.x, accent.y, accent.z, 0.20f));
		draw->AddRect(plate_min, plate_max, ImColor(accent.x, accent.y, accent.z, 0.40f), rounding, ImDrawCornerFlags_All, 1.f);
		draw->AddRectFilled(ImVec2(slide_x - 11.f, plate_max.y - 3.f), ImVec2(slide_x + 11.f, plate_max.y - 1.f), ImColor(accent), 1.f);
	}

	for (size_t i = 0; i < data.size(); i++)
	{
		const std::string id = "##main_tab_" + std::to_string(i);
		const bool pressed_button = ImGui::InvisibleButton(id.c_str(), button_size);
		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();
		const ImVec2 text_size = g_pPrimTextFont ? g_pPrimTextFont->CalcTextSizeA(12.f, FLT_MAX, 0.f, data[i].label.c_str()) : ImGui::CalcTextSize(data[i].label.c_str());
		const bool active = selected == static_cast<int>(i);
		const bool hot = ImGui::IsItemHovered();
		const float fade = data[i].fading_value;
		data[i].active_fade += ((active ? 1.f : 0.f) - data[i].active_fade) * (1.f - expf(-14.f * GImGui->IO.DeltaTime));
		const float on = data[i].active_fade;

		// Inactive: grey that brightens on hover. Active: label white, icon accent blended towards white.
		const ImVec4 idle_col = ImLerp(ImVec4(132 / 255.f, 132 / 255.f, 136 / 255.f, 1.f), ImVec4(196 / 255.f, 196 / 255.f, 200 / 255.f, 1.f), fade);
		const ImVec4 icon_on = ImLerp(accent, ImVec4(1.f, 1.f, 1.f, 1.f), 0.55f);
		const ImColor icon_color(ImLerp(idle_col, icon_on, on));
		const ImColor label_color(ImLerp(idle_col, ImVec4(242 / 255.f, 242 / 255.f, 244 / 255.f, 1.f), on));

		if (fade > 0.f && on < 1.f)
			GImGui->CurrentWindow->DrawList->AddRectFilled(ImVec2(min.x, min.y), ImVec2(max.x, max.y), ImColor(1.f, 1.f, 1.f, 0.06f * fade * (1.f - on)), 10.f);

		const float icon_size = 21.f;
		const ImVec2 center((min.x + max.x) * 0.5f, min.y + 17.f);

		if (i == 6)
		{
			const char* con_txt = ">_";
			const float con_font = 19.f;
			const ImVec2 con_sz = g_pPrimTextFont ? g_pPrimTextFont->CalcTextSizeA(con_font, FLT_MAX, 0.f, con_txt) : ImGui::CalcTextSize(con_txt);
			const ImVec2 con_pos(center.x - con_sz.x * 0.5f, center.y - con_sz.y * 0.5f);

			if (g_pPrimTextFont)
				GImGui->CurrentWindow->DrawList->AddText(g_pPrimTextFont, con_font, con_pos, icon_color, con_txt);
			else
				GImGui->CurrentWindow->DrawList->AddText(con_pos, icon_color, con_txt);
		}
		else
		{
			render::font_icons.draw_icon(GImGui->CurrentWindow->DrawList, icon_chars[i], ImVec2(center.x - icon_size * 0.5f, center.y - icon_size * 0.5f), icon_size, icon_color);
		}
		GImGui->CurrentWindow->DrawList->AddText(g_pPrimTextFont, 12.f, ImVec2((min.x + max.x - text_size.x) * 0.5f, min.y + 31.f), label_color, data[i].label.c_str());

		if (pressed_button)
		{
			changed = selected != static_cast<int>(i);
			selected = static_cast<int>(i);
		}

		if (hot)
		{
			hovered = static_cast<int>(i);

			data[i].fading_value = min(data[i].fading_value + GImGui->IO.DeltaTime * 6.f, 1.00f);
		}
		else
		{
			data[i].fading_value = max(data[i].fading_value - GImGui->IO.DeltaTime * 6.f, 0.00f);

			if (hovered == static_cast<int>(i))
				hovered = -1;
		}

		if (i < data.size() - 1)
		{
			ImGui::SameLine(0.f, gap);
		}
	}

	return changed;
}

bool CMenuGui::TabFeaturesList(std::vector<TabWidgetsData>& data, int& selected, int& hovered)
{
	ImGuiStyle backup_style = GImGui->Style;

	GImGui->Style.ItemSpacing = ImVec2(0, 1);
	GImGui->Style.Colors[ImGuiCol_Border] = ImColor();

	ImVec2 button_size = ImVec2(GImGui->CurrentWindow->Size.x / data.size(), 32);
	ImVec4 button_color = GImGui->Style.Colors[ImGuiCol_ChildBg];
	ImVec4 normal_text_color = GImGui->Style.Colors[ImGuiCol_FrameBgHovered];

	bool changed = false;

	ImGui::PushFont(g_pFontList[Verdana_16px]);

	for (size_t i = 0; i < data.size(); i++)
	{
		GImGui->Style.Colors[ImGuiCol_ButtonActive] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_ButtonHovered] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_Button] = ImLerp(button_color, GImGui->Style.Colors[ImGuiCol_FrameBg], ImSaturate(data[i].fading_value));
		GImGui->Style.Colors[ImGuiCol_Text] = ImLerp(normal_text_color, GImGui->Style.Colors[ImGuiCol_FrameBgActive], ImSaturate(data[i].fading_value));

		bool pressed_button = false;

		if (selected == static_cast<int>(i))
		{
			GImGui->Style.Colors[ImGuiCol_Button] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_ButtonHovered] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_ButtonActive] = GImGui->Style.Colors[ImGuiCol_FrameBg];
			GImGui->Style.Colors[ImGuiCol_Text] = GImGui->Style.Colors[ImGuiCol_FrameBgActive];
			
			pressed_button = ImGui::ButtonTabs(data[i].label.c_str(), button_size);
		}
		else if (hovered == static_cast<int>(i))
		{
			pressed_button = ImGui::ButtonTabs(data[i].label.c_str(), button_size);
		}
		else
		{
			pressed_button = ImGui::ButtonTabs(data[i].label.c_str(), button_size);
		}

		if (pressed_button)
		{
			changed = selected != static_cast<int>(i);
			selected = static_cast<int>(i);
		}

		if (ImGui::IsItemHovered())
		{
			hovered = static_cast<int>(i);

			data[i].fading_value = min(data[i].fading_value + GImGui->IO.DeltaTime * 6.f, 1.00f);
		}
		else
		{
			data[i].fading_value = max(data[i].fading_value - GImGui->IO.DeltaTime * 6.f, 0.00f);

			if (hovered == static_cast<int>(i))
				hovered = -1;
		}

		if (i < data.size() - 1)
			ImGui::SameLine();
	}

	ImGui::PopFont();

	GImGui->Style = backup_style;

	return changed;
}

bool CMenuGui::TabWeaponList(std::vector<TabWidgetsData>& data, int& selected, int& hovered)
{
	ImGuiStyle backup_style = GImGui->Style;

	GImGui->Style.ItemSpacing = ImVec2(4.f, 0.f);
	ImVec2 button_size = ImVec2((GImGui->CurrentWindow->Size.x - 20.f) / data.size(), 24.f);

	bool changed = false;

	for (size_t i = 0; i < data.size(); i++)
	{
		GImGui->Style.Colors[ImGuiCol_Button] = selected == static_cast<int>(i) ? ImColor(193, 154, 164) : ImColor(46, 46, 46);
		GImGui->Style.Colors[ImGuiCol_ButtonHovered] = ImColor(67, 67, 67);
		GImGui->Style.Colors[ImGuiCol_ButtonActive] = ImColor(193, 154, 164);
		const bool pressed_button = ImGui::Button(data[i].label.c_str(), button_size);

		if (pressed_button)
		{
			changed = selected != static_cast<int>(i);
			selected = static_cast<int>(i);
		}

		if (ImGui::IsItemHovered())
		{
			hovered = static_cast<int>(i);

			data[i].fading_value = min(data[i].fading_value + GImGui->IO.DeltaTime * 6.f, 1.00f);
		}
		else
		{
			data[i].fading_value = max(data[i].fading_value - GImGui->IO.DeltaTime * 6.f, 0.00f);

			if (hovered == static_cast<int>(i))
				hovered = -1;
		}

		if (i < data.size() - 1)
			ImGui::SameLine();
	}

	GImGui->Style = backup_style;

	return changed;
}

void CMenuGui::TabSeparator()
{
	ImGuiStyle backup_style = GImGui->Style;

	GImGui->Style.ItemSpacing = ImVec2(0, 1);
	GImGui->Style.Colors[ImGuiCol_Separator] = backup_style.Colors[ImGuiCol_TabActive];

	ImGui::Separator(4);

	GImGui->Style = backup_style;
}

void CMenuGui::TabShadow(const ImVec2& pos, const ImVec2& size, const ImVec4& color)
{
	ImColor border_col_low_alpha = color;
	ImColor border_col_no_alpha = color;

	border_col_low_alpha.value.w = 0.45f;
	border_col_no_alpha.value.w = 0.00f;

	GImGui->CurrentWindow->DrawList->AddRectFilledMultiColor(pos, size, border_col_low_alpha, border_col_low_alpha, border_col_no_alpha, border_col_no_alpha);
}

void CMenuGui::SetID(const char* str_id)
{
	m_id = str_id;
}

void CMenuGui::SetChildSize(ImVec2& size)
{
	size.x = (GImGui->CurrentWindow->Size.x - WINDOW_PADDING_SAMELINE * 2.f - WINDOW_PADDING_SAMELINE * 2.f) / 3.f;
	size.y = GImGui->CurrentWindow->Size.y - ImGui::GetCursorPosY() - WINDOW_PADDING_SAMELINE;
}

void CMenuGui::Text(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ImGui::TextV(fmt, args);
	va_end(args);
}

bool CMenuGui::Checkbox(bool* v, std::string label, std::string message)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::Checkbox(hashed_string.c_str(), v);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::Combo(int* v, std::string label, const char* items[], int size, std::string message, bool owned)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::Combo(hashed_string.c_str(), v, items, size, -1, owned);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::MultiCombo(bool* v, std::string label, const char* items[], int size, std::string message)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::MultiCombo(hashed_string.c_str(), items, v, size);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::Slider(float* v, std::string label, float min, float max, const char* format, float power, std::string message)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);
	
	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::SliderFloat(hashed_string.c_str(), v, min, max, format, power);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::Slider(int* v, std::string label, int min, int max, const char* format, std::string message)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::SliderInt(hashed_string.c_str(), v, min, max, format);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (ImGui::IsItemHovered())
		m_message = message;

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

static bool vector_getter(void* data, int id, const char** out_text)
{
	const auto& vector = *static_cast<std::vector<std::string>*>(data);
	*out_text = vector.at(id).c_str();
	return true;
};

bool CMenuGui::ListBox(const char* label, int* current_item, std::vector<std::string>& values, int height_in_items)
{
	ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetColorU32(ImGuiCol_FrameBg));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetColorU32(ImGuiCol_FrameBg));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetColorU32(ImGuiCol_FrameBg));

	const bool ret = ImGui::ListBox(label, current_item, vector_getter, &values, values.size(), height_in_items);

	ImGui::PopStyleColor(3);

	return ret;
}

void CMenuGui::Separator()
{
	ImGui::Spacing();
	ImGui::Separator(GImGui->CurrentWindow->Size.x - WINDOW_PADDING_SAMELINE);
	ImGui::Spacing();
}

void CMenuGui::Link(std::string url)
{
	static std::vector<LinkData> url_list;

	int id = -1;

	if (!url_list.empty())
	{
		for (size_t i = 0; i < url_list.size(); i++)
		{
			if (url_list[i].url.compare(url))
				continue;

			id = static_cast<int>(i);
			break;
		}
	}

	if (id == -1)
	{
		url_list.push_back(LinkData{ url, false, false });
		id = static_cast<int>(url_list.size()) - 1;
	}

	bool ret = false;

	for (size_t i = 0; i < url_list.size(); i++)
	{
		if (id == static_cast<int>(i))
			continue;

		if (url_list[i].hovered || url_list[i].clicked)
			ret = true;
	}

	ImGui::Text(url.c_str());

	if (ret)
		return;

	url_list[id].hovered = ImGui::IsItemHovered();

	if (url_list[id].hovered)
	{
		ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

		GImGui->CurrentWindow->Flags |= ImGuiWindowFlags_NoMove;

		if (ImGui::IsItemClicked(0))
			url_list[id].clicked = true;

		if (url_list[id].clicked && ImGui::IsMouseReleased(0))
		{
			ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_SHOW);
			url_list[id].clicked = false;
		}
	}
	else
	{
		GImGui->CurrentWindow->Flags &= ~ImGuiWindowFlags_NoMove;

		url_list[id].clicked = false;
	}
}

#define WAIT_FOR_KEY(wait, assign)\
if (GetAsyncKeyState(wait) & 0x8000) *keynum = assign;

static void WaitForPressKey(int* keynum)
{
	WAIT_FOR_KEY(VK_F1, K_F1);
	WAIT_FOR_KEY(VK_F2, K_F2);
	WAIT_FOR_KEY(VK_F3, K_F3);
	WAIT_FOR_KEY(VK_F4, K_F4);
	WAIT_FOR_KEY(VK_F5, K_F5);
	WAIT_FOR_KEY(VK_F6, K_F6);
	WAIT_FOR_KEY(VK_F7, K_F7);
	WAIT_FOR_KEY(VK_F8, K_F8);
	WAIT_FOR_KEY(VK_F9, K_F9);
	WAIT_FOR_KEY(VK_F10, K_F10);
	WAIT_FOR_KEY(VK_F11, K_F11);
	WAIT_FOR_KEY(VK_F12, K_F12);
	WAIT_FOR_KEY(0x30, K_0);
	WAIT_FOR_KEY(0x31, K_1);
	WAIT_FOR_KEY(0x32, K_2);
	WAIT_FOR_KEY(0x33, K_3);
	WAIT_FOR_KEY(0x34, K_4);
	WAIT_FOR_KEY(0x35, K_5);
	WAIT_FOR_KEY(0x36, K_6);
	WAIT_FOR_KEY(0x37, K_7);
	WAIT_FOR_KEY(0x38, K_8);
	WAIT_FOR_KEY(0x39, K_9);
	WAIT_FOR_KEY(VK_NUMPAD0, K_KP_INS);
	WAIT_FOR_KEY(VK_NUMPAD1, K_KP_END);
	WAIT_FOR_KEY(VK_NUMPAD2, K_KP_DOWNARROW);
	WAIT_FOR_KEY(VK_NUMPAD3, K_KP_PGDN);
	WAIT_FOR_KEY(VK_NUMPAD4, K_KP_LEFTARROW);
	WAIT_FOR_KEY(VK_NUMPAD5, K_KP_5);
	WAIT_FOR_KEY(VK_NUMPAD6, K_KP_RIGHTARROW);
	WAIT_FOR_KEY(VK_NUMPAD7, K_KP_HOME);
	WAIT_FOR_KEY(VK_NUMPAD8, K_KP_UPARROW);
	WAIT_FOR_KEY(VK_NUMPAD9, K_KP_PGUP);
	WAIT_FOR_KEY(VK_SPACE, K_SPACE);
	WAIT_FOR_KEY(VK_RETURN, K_ENTER);
	WAIT_FOR_KEY(VK_TAB, K_TAB);
	WAIT_FOR_KEY(VK_UP, K_UPARROW);
	WAIT_FOR_KEY(VK_DOWN, K_DOWNARROW);
	WAIT_FOR_KEY(VK_LEFT, K_LEFTARROW);
	WAIT_FOR_KEY(VK_RIGHT, K_RIGHTARROW);
	WAIT_FOR_KEY(18, K_ALT);
	WAIT_FOR_KEY(VK_CONTROL, K_CTRL);
	WAIT_FOR_KEY(VK_SHIFT, K_SHIFT);
	WAIT_FOR_KEY(34, K_PGDN);
	WAIT_FOR_KEY(33, K_PGUP);
	WAIT_FOR_KEY(VK_HOME, K_HOME);
	WAIT_FOR_KEY(VK_END, K_END);
	WAIT_FOR_KEY(0x41, K_A);
	WAIT_FOR_KEY(0x42, K_B);
	WAIT_FOR_KEY(0x43, K_C); 
	WAIT_FOR_KEY(0x44, K_D);
	WAIT_FOR_KEY(0x45, K_E);
	WAIT_FOR_KEY(0x46, K_F);
	WAIT_FOR_KEY(0x47, K_G);
	WAIT_FOR_KEY(0x48, K_H);
	WAIT_FOR_KEY(0x49, K_I);
	WAIT_FOR_KEY(0x4A, K_J);
	WAIT_FOR_KEY(0x4B, K_K);
	WAIT_FOR_KEY(0x4C, K_L);
	WAIT_FOR_KEY(0x4D, K_M);
	WAIT_FOR_KEY(0x4E, K_N);
	WAIT_FOR_KEY(0x4F, K_O);
	WAIT_FOR_KEY(0x50, K_P);
	WAIT_FOR_KEY(0x51, K_Q);
	WAIT_FOR_KEY(0x52, K_R);
	WAIT_FOR_KEY(0x53, K_S);
	WAIT_FOR_KEY(0x54, K_T);
	WAIT_FOR_KEY(0x55, K_U);
	WAIT_FOR_KEY(0x56, K_V);
	WAIT_FOR_KEY(0x57, K_W);
	WAIT_FOR_KEY(0x58, K_X);
	WAIT_FOR_KEY(0x59, K_Y);
	WAIT_FOR_KEY(0x5A, K_Z);
	WAIT_FOR_KEY(0xDB, K_BL);
	WAIT_FOR_KEY(0xDD, K_BR);
	WAIT_FOR_KEY(0xBC, K_COMMA);
	WAIT_FOR_KEY(0xBE, K_POINT);
	WAIT_FOR_KEY(VK_LBUTTON, K_MOUSE1);
	WAIT_FOR_KEY(VK_RBUTTON, K_MOUSE2);
	WAIT_FOR_KEY(VK_MBUTTON, K_MOUSE3);
	WAIT_FOR_KEY(VK_XBUTTON1, K_MOUSE4);
	WAIT_FOR_KEY(VK_XBUTTON2, K_MOUSE5);
	
	
}

bool CMenuGui::Key(void* v, std::string label, bool owned)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::KeyButton(hashed_string.c_str(), v, owned);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::KeyToggle(void* v, std::string label, bool owned)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::KeyButtonToggle(hashed_string.c_str(), v, owned);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::KeyPress(void* v, std::string label, bool owned)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::KeyButtonPress(hashed_string.c_str(), v, owned);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

bool CMenuGui::ColorEdit(float* v, std::string label)
{
	std::string hashed_string = label + m_id;

	float& fading_value = FindWidgetsFadingValue(hashed_string);

	ImGui::PushStyleColor(ImGuiCol_Border, ImLerp(GImGui->Style.Colors[ImGuiCol_ChildBg],
		GImGui->Style.Colors[ImGuiCol_Border], ImSaturate(fading_value)));

	const bool ret = ImGui::ColorEdit(hashed_string.c_str(), v);

	ImGui::PopStyleColor();

	UpdateWidgetsFadingValue(fading_value);

	if (m_check_change_value && !m_changed_value)
		m_changed_value = ret;

	return ret;
}

void CMenuGui::BeginCheckChangeValue()
{
	if (!m_check_change_value)
		m_check_change_value = true;
}

void CMenuGui::EndCheckChangeValue()
{
	if (m_check_change_value)
	{
		m_check_change_value = false;
		m_changed_value = false;
	}
}

bool CMenuGui::IsChangedValue()
{
	return m_changed_value;
}

float& CMenuGui::FindWidgetsFadingValue(std::string label)
{
	auto it = m_widgets_list.find(label);

	if (it == m_widgets_list.end())
	{
		m_widgets_list.insert({ label, 0.00f });
		it = m_widgets_list.find(label);
	}

	return it->second;
}

void CMenuGui::UpdateWidgetsFadingValue(float& fading_value)
{
	if (ImGui::IsItemHovered())
		fading_value = min(fading_value + GImGui->IO.DeltaTime * 6.f, 1.00f);
	else
		fading_value = max(fading_value - GImGui->IO.DeltaTime * 6.f, 0.00f);
}

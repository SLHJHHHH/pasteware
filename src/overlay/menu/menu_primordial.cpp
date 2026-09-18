#include "framework.h"
#include "overlay/font_atlas.h"
#include "overlay/menu/gui/components.h"

namespace
{
	static ImU32 accent_u32(float alpha);
	static ImU32 white_u32(float alpha);

	static bool contains_filter(const char* text, const char* filter)
	{
		if (!filter || !filter[0])
			return true;

		const size_t len = strlen(filter);

		for (const char* p = text; *p; ++p)
			if (!_strnicmp(p, filter, len))
				return true;

		return false;
	}

	void panel(const char* name, const ImVec2& pos, const ImVec2& size, const std::function<void()>& draw, bool glow = false)
	{
		if (g_pMenu.get() && !contains_filter(name, g_pMenu->SearchText()))
			return;

		ImGui::SetCursorPos(pos);
		if (glow)
		{
			const ImVec2 g0 = ImGui::GetCursorScreenPos();
			ImDrawList* gdl = ImGui::GetWindowDrawList();
			gdl->AddRect(g0 - ImVec2(5.f, 5.f), g0 + size + ImVec2(5.f, 5.f), accent_u32(0.06f), 12.f, 15, 6.f);
			gdl->AddRect(g0 - ImVec2(2.f, 2.f), g0 + size + ImVec2(2.f, 2.f), accent_u32(0.10f), 10.f, 15, 1.5f);
		}
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.095f, 0.095f, 0.135f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.32f, 0.30f, 0.46f, 0.55f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 11.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.f);
		ImGui::BeginChild(name, size, true);
		{
			const ImVec2 t0 = ImGui::GetCursorScreenPos();
			ImDrawList* dl = ImGui::GetWindowDrawList();
			dl->AddCircleFilled(t0 + ImVec2(3.f, 7.f), 3.f, accent_u32(0.95f));
			dl->AddCircle(t0 + ImVec2(3.f, 7.f), 5.5f, accent_u32(0.25f), 16, 1.2f);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 12.f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96f, 0.95f, 0.98f, 1.f));
			ImGui::TextUnformatted(name);
			ImGui::PopStyleColor();
			const ImVec2 tmin = ImGui::GetItemRectMin();
			const ImVec2 tmax = ImGui::GetItemRectMax();
			const float ly = tmax.y + 2.f;
			dl->AddLine(ImVec2(tmin.x, ly), ImVec2(tmin.x + (tmax.x - tmin.x) * 0.5f), accent_u32(0.90f), 2.f);
			dl->AddLine(ImVec2(tmin.x, ly), ImVec2(tmax.x, ly), white_u32(0.06f), 1.f);
			ImGui::Spacing();
			ImGui::Spacing();
			draw();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);
	}

	bool side(const char* label, bool active)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, active ? ImVec4(1.0f, 0.18f, 0.55f, 0.22f) : ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.18f, 0.55f, 0.26f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 0.22f, 0.60f, 0.36f));

		ImGui::PushStyleColor(ImGuiCol_Text, active ? ImVec4(1.0f, 1.0f, 1.0f, 1.f) : ImVec4(0.66f, 0.66f, 0.72f, 1.f));
		const bool hit = ImGui::Button(label, ImVec2(134.f, 32.f));
		if (active)
		{
			const ImVec2 mn = ImGui::GetItemRectMin();
			const ImVec2 mx = ImGui::GetItemRectMax();
			ImDrawList* dl = GImGui->CurrentWindow->DrawList;
			dl->AddRectFilled(mn, mn + ImVec2(3.f, 32.f), accent_u32(1.f), 2.f);
			dl->AddRectFilled(mn + ImVec2(3.f, 0.f), mn + ImVec2(14.f, 32.f), accent_u32(0.16f));
			dl->AddRect(mn, mx, accent_u32(0.30f), 5.f, 15, 1.f);
		}
		ImGui::PopStyleColor(4);
		return hit;
	}

	void tabs(const std::vector<const char*>& labels, int& selected)
	{
		const char* filter = g_pMenu.get() ? g_pMenu->SearchText() : nullptr;

		for (size_t i = 0; i < labels.size(); ++i)
		{
			if (filter && filter[0] && !contains_filter(labels[i], filter))
				continue;

			if (side(labels[i], selected == static_cast<int>(i)))
				selected = static_cast<int>(i);
		}
	}

	static ImU32 accent_u32(float alpha)
	{
		ImVec4 c = components::get_accent_color();
		c.w = alpha;
		return ImGui::GetColorU32(c);
	}

	static ImU32 white_u32(float alpha)
	{
		return ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, alpha));
	}

	static void draw_aa_radar(int side_mode, float max_radius = 92.f)
	{
		const ImVec2 cur = ImGui::GetCursorScreenPos();
		const ImVec2 avail = ImGui::GetContentRegionAvail();
		const float radius = ImMax(ImMin(avail.x * 0.5f - 10.f, max_radius), 24.f);
		const ImVec2 c = cur + ImVec2(avail.x * 0.5f, radius + 8.f);
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const float t = static_cast<float>(ImGui::GetTime());

		dl->AddCircleFilled(c, radius, white_u32(0.020f), 64);
		dl->AddCircle(c, radius, white_u32(0.07f), 64, 1.f);
		dl->AddCircle(c, radius * 0.72f, white_u32(0.06f), 64, 1.f);
		dl->AddCircle(c, radius * 0.44f, white_u32(0.05f), 64, 1.f);
		dl->AddLine(c - ImVec2(radius, 0.f), c + ImVec2(radius, 0.f), white_u32(0.05f), 1.f);
		dl->AddLine(c - ImVec2(0.f, radius), c + ImVec2(0.f, radius), white_u32(0.05f), 1.f);

		for (int i = 0; i < 12; ++i)
		{
			const float a = static_cast<float>(i) * (6.28318f / 12.f);
			const ImVec2 p0 = c + ImVec2(ImCos(a) * (radius - 6.f), ImSin(a) * (radius - 6.f));
			const ImVec2 p1 = c + ImVec2(ImCos(a) * radius, ImSin(a) * radius);
			dl->AddLine(p0, p1, white_u32(0.05f), 1.f);
		}

		const float ang = t * 0.8f;
		const ImVec2 tip = c + ImVec2(ImCos(ang) * radius, ImSin(ang) * radius);
		dl->AddLine(c, tip, accent_u32(0.85f), 2.f);
		dl->AddCircle(tip, 8.f, accent_u32(0.30f), 24, 1.5f);
		dl->AddCircleFilled(tip, 3.5f, accent_u32(1.f));

		const float fake = ang + (side_mode == 1 ? -0.7f : side_mode == 2 ? 0.7f : 0.7f);
		const ImVec2 ftip = c + ImVec2(ImCos(fake) * radius * 0.72f, ImSin(fake) * radius * 0.72f);
		dl->AddLine(c, ftip, ImGui::GetColorU32(ImVec4(0.35f, 0.75f, 1.f, 0.75f)), 1.5f);
		dl->AddCircleFilled(ftip, 2.5f, ImGui::GetColorU32(ImVec4(0.35f, 0.75f, 1.f, 1.f)));
		dl->AddCircleFilled(c, 4.5f, white_u32(0.90f));

		ImGui::Dummy(ImVec2(avail.x, radius * 2.f + 16.f));
	}

	static void draw_latency_meter(int latency)
	{
		const float t = static_cast<float>(ImGui::GetTime());
		const int bars = 30;
		const float avail = ImGui::GetContentRegionAvail().x;
		const float bar_w = ImMax((avail - static_cast<float>(bars - 1) * 3.f) / static_cast<float>(bars), 2.f);
		const float max_h = 44.f;
		const ImVec2 start = ImGui::GetCursorScreenPos();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const int lit = ImMin(ImMax(latency, 0) / 16, bars);

		for (int i = 0; i < bars; ++i)
		{
			const float phase = ImSin(t * 2.2f + static_cast<float>(i) * 0.45f) * 0.5f + 0.5f;
			float h = 8.f + phase * (max_h - 12.f);
			const bool on = i < lit;
			if (on)
				h = ImMin(h + 8.f, max_h);
			const float x0 = start.x + static_cast<float>(i) * (bar_w + 3.f);
			dl->AddRectFilled(ImVec2(x0, start.y), ImVec2(x0 + bar_w, start.y + max_h), white_u32(0.035f), 2.f);
			dl->AddRectFilled(ImVec2(x0, start.y + max_h - h), ImVec2(x0 + bar_w, start.y + max_h),
				on ? accent_u32(0.95f) : white_u32(0.13f), 2.f);
		}
		ImGui::Dummy(ImVec2(avail, max_h + 4.f));
	}

	static void draw_tick_bar(const char* label, int value, int max, const char* suffix)
	{
		const int cells = ImMax(max, 1);
		const float t = static_cast<float>(ImGui::GetTime());
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.78f, 0.78f, 0.85f, 1.f));
		ImGui::Text("%s", label);
		ImGui::PopStyleColor();
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.55f, 0.75f, 0.95f));
		ImGui::Text("%d %s", value, suffix);
		ImGui::PopStyleColor();

		const int show = ImMin(cells, 40);
		const ImVec2 start = ImGui::GetCursorScreenPos();
		const float avail = ImGui::GetContentRegionAvail().x;
		const float cell_w = ImMax(ImMin(14.f, (avail - static_cast<float>(show - 1) * 2.f) / static_cast<float>(show)), 1.5f);
		ImDrawList* dl = ImGui::GetWindowDrawList();
		for (int i = 0; i < show; ++i)
		{
			const float scaled = static_cast<float>(i + 1) * static_cast<float>(cells) / static_cast<float>(show);
			const bool on = scaled <= static_cast<float>(value);
			const bool tip = on && value < cells && scaled + static_cast<float>(cells) / static_cast<float>(show) > static_cast<float>(value);
			const ImVec2 c0 = start + ImVec2(static_cast<float>(i) * (cell_w + 2.f), 2.f);
			const float pulse = tip ? 0.55f + 0.45f * ImSin(t * 5.f) : 1.f;
			dl->AddRectFilled(c0, c0 + ImVec2(cell_w, 8.f),
				on ? accent_u32(0.95f * pulse) : white_u32(0.07f), 2.f);
		}
		ImGui::Dummy(ImVec2(avail, 12.f));
	}

	static void draw_status_row(const char* label, bool on)
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImVec2 p = ImGui::GetCursorScreenPos() + ImVec2(4.f, 7.f);
		dl->AddCircleFilled(p, 3.f, on ? ImGui::GetColorU32(ImVec4(0.35f, 0.95f, 0.55f, 1.f)) : ImGui::GetColorU32(ImVec4(0.45f, 0.45f, 0.52f, 1.f)));
		if (on)
			dl->AddCircle(p, 6.f, ImGui::GetColorU32(ImVec4(0.35f, 0.95f, 0.55f, 0.25f)), 20, 1.2f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 14.f);
		ImGui::PushStyleColor(ImGuiCol_Text, on ? ImVec4(0.90f, 0.90f, 0.94f, 1.f) : ImVec4(0.55f, 0.55f, 0.62f, 1.f));
		ImGui::TextUnformatted(label);
		ImGui::PopStyleColor();
	}

	struct bind_row_t
	{
		const char* label;
		bind_t* bind;
	};

	const bind_row_t g_bind_rows[] =
	{
		{ "Legit aim", &cvars::legitbot.aim_key },
		{ "Legit psilent", &cvars::legitbot.aim_psilent_key },
		{ "Triggerbot", &cvars::legitbot.trigger_key },
		{ "Rage aim", &cvars::ragebot.raim_key },
		{ "Force body", &cvars::ragebot.raim_force_body_key },
		{ "Force damage", &cvars::ragebot.raim_force_min_damage_key },
		{ "Antiaim side", &cvars::ragebot.aa_side_key },
		{ "Knifebot", &cvars::misc.kb_key },
		{ "Thirdperson", &cvars::visuals.effects_thirdperson_key },
		{ "Panic", &cvars::visuals.panic_key },
		{ "Fake latency", &cvars::misc.fakelatency_key },
		{ "Auto block", &cvars::misc.auto_block_key },
		{ "Auto boost", &cvars::misc.auto_boost_key },
		{ "Game speed", &cvars::misc.speed_key },
		{ "Airstuck", &cvars::misc.airstuck_key },
		{ "Doubletap", &cvars::misc.doubletap_key },
		{ "Kreedz", &cvars::kreedz.key },
		{ "Fast run", &cvars::kreedz.fastrun_key },
		{ "Slow walk", &cvars::kreedz.slowwalk_key },
		{ "DeGen", &cvars::kreedz.degen_key }
	};

	const int g_global_ids[] = { WEAPON_NONE };
	const char* g_global_names[] = { "Global" };
	const int g_pistol_ids[] = { WEAPON_P228, WEAPON_GLOCK, WEAPON_GLOCK18, WEAPON_USP, WEAPON_DEAGLE, WEAPON_FIVESEVEN, WEAPON_ELITE };
	const char* g_pistol_names[] = { "P228", "Glock", "Glock18", "USP", "Deagle", "FiveSeven", "Elite" };
	const int g_smg_ids[] = { WEAPON_MAC10, WEAPON_TMP, WEAPON_MP5N, WEAPON_UMP45, WEAPON_P90 };
	const char* g_smg_names[] = { "Mac10", "TMP", "MP5", "UMP45", "P90" };
	const int g_rifle_ids[] = { WEAPON_GALIL, WEAPON_FAMAS, WEAPON_AK47, WEAPON_M4A1, WEAPON_SG552, WEAPON_AUG, WEAPON_M249 };
	const char* g_rifle_names[] = { "Galil", "Famas", "AK47", "M4A1", "SG552", "AUG", "M249" };
	const int g_shotgun_ids[] = { WEAPON_M3, WEAPON_XM1014 };
	const char* g_shotgun_names[] = { "M3", "XM1014" };
	const int g_sniper_ids[] = { WEAPON_SCOUT, WEAPON_AWP, WEAPON_SG550, WEAPON_G3SG1 };
	const char* g_sniper_names[] = { "Scout", "AWP", "SG550", "G3SG1" };
	void CopyProfile(int from, int to)
	{
		cvars::weapons[to] = cvars::weapons[from];
	}
}

void CMenu::Draw()
{
	enum { AIMBOT, ANTIAIM, VISUALS, MOVEMENT, MISC, CONFIGS, CONSOLE };
	static int section[7]{};
	static int weapon_group{};
	static int current_weapon = WEAPON_NONE;
	static int profile_mode{};
	static int profile_item{};
	static std::vector<TabWidgetsData> main = { 
		{ "Aimbot", 0.f }, 
		{ "Antiaim", 0.f }, 
		{ "Visuals", 0.f }, 
		{ "Movement", 0.f }, 
		{ "Misc", 0.f }, 
		{ "Config", 0.f }, 
		{ "Console", 0.f } 
	};

	if (!m_bIsOpened)
		return;

	if (!m_pGui->Begin("##pasteware", ImVec2(876.f, 623.f)))
		return;

	m_WindowPos = GImGui->CurrentWindow->Pos;
	m_WindowSize = GImGui->CurrentWindow->Size;

	ImVec4 accent = components::get_accent_color();

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 10.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.f, 4.f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.91f, 0.91f, 0.95f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.47f, 0.47f, 0.56f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.135f, 0.135f, 0.20f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.18f, 0.18f, 0.27f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.23f, 0.21f, 0.33f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(accent.x, accent.y, accent.z, 1.f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(accent.x, accent.y, accent.z, 1.f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(ImMin(accent.x * 1.15f, 1.f), ImMin(accent.y * 1.25f, 1.f), ImMin(accent.z * 1.15f, 1.f), 1.f));
	
	if (g_pPrimTextFont)
		ImGui::PushFont(g_pPrimTextFont);
	
	m_pGui->TabBackground();
	m_pGui->TabList(main, m_iSelectedTab, m_iHoveredTab);

	const float content_h = m_WindowSize.y - 70.f - 75.f;
	const float content_w = m_WindowSize.x - 153.f - 15.f;
	const float panel_w = (content_w - 15.f) * 0.5f;
	const ImVec2 a(0.f, 0.f), b(panel_w + 15.f, 0.f);

	ImGui::SetCursorPos(ImVec2(5.f, 70.f));
	ImGui::BeginChild("##prim_sidebar", ImVec2(136.f, content_h), false);
	switch (m_iSelectedTab)
	{
	case AIMBOT: tabs({ "Legit", "Rage", "Trigger", "Weapon" }, section[AIMBOT]); break;
	case ANTIAIM: tabs({ "Global", "Stand", "Move", "Fakelag" }, section[ANTIAIM]); break;
	case VISUALS: tabs({ "ESP", "Chams", "Local", "World" }, section[VISUALS]); break;
	case MOVEMENT: tabs({ "Main", "Strafes", "Bugs" }, section[MOVEMENT]); break;
	case MISC: tabs({ "Exploits", "Automation", "Knifebot", "Other", "Theme" }, section[MISC]); break;
	case CONFIGS: tabs({ "Configurations", "Binds" }, section[CONFIGS]); break;
	case CONSOLE: tabs({ "Console" }, section[CONSOLE]); break;
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(153.f, 70.f));
	ImGui::BeginChild("##prim_content", ImVec2(content_w, content_h), false);

	if (m_iSelectedTab == AIMBOT)
	{
		if (section[AIMBOT] == 0)
		{
			float ya = a.y;
			float yb = b.y;
			panel("Primary", ImVec2(a.x, ya), ImVec2(panel_w, 330.f), [&]()
			{
				components::checkbox("Enabled", &cvars::legitbot.active, &cvars::legitbot.aim_key);
				components::keybind("Aim key", &cvars::legitbot.aim_key);
				components::keybind("Silent key", &cvars::legitbot.aim_psilent_key);
				components::checkbox("Friendly fire", &cvars::legitbot.friendly_fire);
				components::checkbox("Automatic scope", &cvars::legitbot.aim_auto_scope);
				components::checkbox("Smoke check", &cvars::legitbot.aim_smoke_check);
				components::checkbox("Dynamic FOV", &cvars::legitbot.aim_dynamic_fov);
			});
			ya += 338.f;
			panel("Recoil", ImVec2(a.x, ya), ImVec2(panel_w, 170.f), [&]()
			{
				components::checkbox("Return angles", &cvars::legitbot.aim_recoil_return_angles);
				components::checkbox("No mousemove trigger", &cvars::legitbot.aim_recoil_no_mousemove_trigger);
				components::checkbox("Disable smooth auto", &cvars::legitbot.aim_recoil_disable_smooth_auto);
			});
			ya += 178.f;
			panel("Protections", ImVec2(a.x, ya), ImVec2(panel_w, 480.f), [&]()
			{
				components::checkbox("Position adjustment", &cvars::legitbot.position_adjustment);
				components::checkbox("Desync helper", &cvars::legitbot.desync_helper);
				components::slider_int("Flashed check", &cvars::legitbot.aim_flashed_check, 0, 100, "%d", "%");
				components::checkbox("Shoot through teammates", &cvars::legitbot.aim_shoot_through_teammates);
				components::checkbox("Dont shoot in shield", &cvars::legitbot.aim_dont_shoot_in_shield);
				components::checkbox("Demochecker bypass", &cvars::legitbot.aim_demochecker_bypass);
				components::slider_int("Block attack after kill", &cvars::legitbot.aim_block_attack_after_kill, 0, 10, "%d", "");
				components::checkbox("Pseudo professional", &cvars::legitbot.aim_pseudo_professional);
				components::checkbox("Demo visible only", &cvars::legitbot.aim_write_demo_visible_only);
				components::checkbox("Mouse event", &cvars::legitbot.aim_mouse_event);
				components::checkbox("FOV scale by FPS", &cvars::legitbot.aim_fov_scale_by_fps);
				components::checkbox("Smooth independence FPS", &cvars::legitbot.aim_smooth_independence_fps);
			});
			panel("Target Weights", ImVec2(b.x, yb), ImVec2(panel_w, 330.f), [&]()
			{
				components::slider_int("Head weight", &cvars::legitbot.target_weight_head, 1, 100, "%d", "%");
				components::slider_int("Neck weight", &cvars::legitbot.target_weight_neck, 1, 100, "%d", "%");
				components::slider_int("Chest weight", &cvars::legitbot.target_weight_chest, 1, 100, "%d", "%");
				components::slider_int("Stomach weight", &cvars::legitbot.target_weight_stomach, 1, 100, "%d", "%");
				components::slider_int("Arms weight", &cvars::legitbot.target_weight_arms, 1, 100, "%d", "%");
				components::slider_int("Legs weight", &cvars::legitbot.target_weight_legs, 1, 100, "%d", "%");
			});
			yb += 338.f;
			panel("Targeting", ImVec2(b.x, yb), ImVec2(panel_w, 150.f), [&]()
			{
				const char* sel[] = { "Closest FOV", "Hitbox weight" };
				components::combo("Aim selection", &cvars::legitbot.target_aim_selection, sel, IM_ARRAYSIZE(sel));
				components::slider_int("Switch delay", &cvars::legitbot.target_switch_delay, 0, 1000, "%d", "ms");
			});
		}
		else if (section[AIMBOT] == 1)
		{
			float ya = a.y;
			float yb = b.y;
			panel("Ragebot", ImVec2(a.x, ya), ImVec2(panel_w, 300.f), [&]()
			{
				components::checkbox("Enabled", &cvars::ragebot.active, &cvars::ragebot.raim_key);
				components::keybind("Aim key", &cvars::ragebot.raim_key);
				components::checkbox("Friendly fire", &cvars::ragebot.friendly_fire);
				components::checkbox("Automatic fire", &cvars::ragebot.raim_auto_fire);
				components::slider_float("Maximum FOV", &cvars::ragebot.raim_fov, 0.f, 180.f, "%.1f", "°");
			});
			ya += 308.f;
			panel("Accuracy", ImVec2(a.x, ya), ImVec2(panel_w, 400.f), [&]()
			{
				const char* type[] = { "Legit AA correction", "Silent", "Teleport" };
				components::combo("Aim type", &cvars::ragebot.raim_type, type, IM_ARRAYSIZE(type));
				components::checkbox("Tapping mode", &cvars::ragebot.raim_tapping_mode);
				components::keybind("Force damage key", &cvars::ragebot.raim_force_min_damage_key);
				components::slider_int("Force damage", &cvars::ragebot.raim_force_min_damage_value, 1, 100, "%d", "hp");
				const char* delay[] = { "Unlag", "History" };
				components::multi_combo("Delayshot", cvars::ragebot.raim_delayshot, delay, IM_ARRAYSIZE(delay));
				components::keybind("Force body key", &cvars::ragebot.raim_force_body_key);
				components::checkbox("Remove recoil", &cvars::ragebot.raim_remove_recoil);
				const char* spread[] = { "Off", "Pitch + yaw", "Pitch + roll", "Pitch + yaw + roll" };
				components::combo("Remove spread", &cvars::ragebot.raim_remove_spread, spread, IM_ARRAYSIZE(spread));
				components::checkbox("Auto scope", &cvars::ragebot.raim_auto_scope);
			});
			ya += 408.f;
			panel("Mitigations", ImVec2(a.x, ya), ImVec2(panel_w, 170.f), [&]()
			{
				components::slider_float("Low FPS value", &cvars::ragebot.raim_low_fps_value, 5.f, 500.f, "%.0f", "fps");
				const char* mit[] = { "Traces only if can attack", "Dont trace arms", "Dont trace legs" };
				components::multi_combo("Mitigations", cvars::ragebot.raim_low_fps_mitigations, mit, IM_ARRAYSIZE(mit));
			});
			panel("Targeting", ImVec2(b.x, yb), ImVec2(panel_w, 470.f), [&]()
			{
				const char* tsel[] = { "Highest damage", "Closest by FOV", "Hitbox weights" };
				components::multi_combo("Selection", cvars::ragebot.raim_target_selection, tsel, IM_ARRAYSIZE(tsel));
				components::slider_int("Damage weight", &cvars::ragebot.raim_target_weight_damage, 0, 100, "%d", "%");
				components::slider_int("FOV weight", &cvars::ragebot.raim_target_weight_fov, 0, 100, "%d", "%");
				components::slider_int("Head weight", &cvars::ragebot.raim_target_weight_head, 1, 100, "%d", "%");
				components::slider_int("Neck weight", &cvars::ragebot.raim_target_weight_neck, 1, 100, "%d", "%");
				components::slider_int("Chest weight", &cvars::ragebot.raim_target_weight_chest, 1, 100, "%d", "%");
				components::slider_int("Stomach weight", &cvars::ragebot.raim_target_weight_stomach, 1, 100, "%d", "%");
				components::slider_int("Arms weight", &cvars::ragebot.raim_target_weight_arms, 1, 100, "%d", "%");
				components::slider_int("Legs weight", &cvars::ragebot.raim_target_weight_legs, 1, 100, "%d", "%");
			});
			yb += 478.f;
			panel("Resolver", ImVec2(b.x, yb), ImVec2(panel_w, 150.f), [&]()
			{
				const char* pitch[] = { "Off", "Resolve untrusted" };
				components::combo("Pitch", &cvars::ragebot.raim_resolver_pitch, pitch, IM_ARRAYSIZE(pitch));
				components::text("GLOBAL");
			});
		}
		else if (section[AIMBOT] == 2)
		{
			panel("Triggerbot", a, ImVec2(panel_w, 360.f), [&]()
			{
				components::keybind("Trigger key", &cvars::legitbot.trigger_key);
				components::keybind("Silent key", &cvars::legitbot.aim_psilent_key);
				components::checkbox("Accurate traces", &cvars::legitbot.trigger_accurate_traces);
				components::checkbox("Only scoped", &cvars::legitbot.trigger_only_scoped);
				components::checkbox("Dynamic", &cvars::legitbot.trigger_dynamic);
				components::checkbox("Off after first bullet", &cvars::legitbot.trigger_turn_off_after_first_bullet);
				components::slider_int("Shot delay", &cvars::legitbot.trigger_shot_delay, 0, 1000, "%d", "ms");
			});
			panel("Hitbox Scale", b, ImVec2(panel_w, 560.f), [&]()
			{
				components::slider_float("Head scale", &cvars::legitbot.trigger_hitbox_scale[0], 0.f, 200.f, "%.0f", "%");
				components::text(cvars::legitbot.trigger_hitbox_scale[0] > 0.5f ? "Head: ON" : "Head: OFF");
				components::slider_float("Neck scale", &cvars::legitbot.trigger_hitbox_scale[1], 0.f, 200.f, "%.0f", "%");
				components::text(cvars::legitbot.trigger_hitbox_scale[1] > 0.5f ? "Neck: ON" : "Neck: OFF");
				components::slider_float("Chest scale", &cvars::legitbot.trigger_hitbox_scale[2], 0.f, 200.f, "%.0f", "%");
				components::text(cvars::legitbot.trigger_hitbox_scale[2] > 0.5f ? "Chest: ON" : "Chest: OFF");
				components::slider_float("Stomach scale", &cvars::legitbot.trigger_hitbox_scale[3], 0.f, 200.f, "%.0f", "%");
				components::text(cvars::legitbot.trigger_hitbox_scale[3] > 0.5f ? "Stomach: ON" : "Stomach: OFF");
				components::slider_float("Arms scale", &cvars::legitbot.trigger_hitbox_scale[4], 0.f, 200.f, "%.0f", "%");
				components::text(cvars::legitbot.trigger_hitbox_scale[4] > 0.5f ? "Arms: ON" : "Arms: OFF");
				components::slider_float("Legs scale", &cvars::legitbot.trigger_hitbox_scale[5], 0.f, 200.f, "%.0f", "%");
				components::text(cvars::legitbot.trigger_hitbox_scale[5] > 0.5f ? "Legs: ON" : "Legs: OFF");
			});
		}
		else
		{
			const int* gids = g_global_ids;
			const char** gnames = (const char**)g_global_names;
			int gcount = 1;
			switch (weapon_group)
			{
			case 1:
				gids = g_pistol_ids;
				gnames = (const char**)g_pistol_names;
				gcount = 7;
				break;
			case 2:
				gids = g_smg_ids;
				gnames = (const char**)g_smg_names;
				gcount = 5;
				break;
			case 3:
				gids = g_rifle_ids;
				gnames = (const char**)g_rifle_names;
				gcount = 7;
				break;
			case 4:
				gids = g_shotgun_ids;
				gnames = (const char**)g_shotgun_names;
				gcount = 2;
				break;
			case 5:
				gids = g_sniper_ids;
				gnames = (const char**)g_sniper_names;
				gcount = 4;
				break;
			}
			if (profile_item >= gcount)
				profile_item = 0;
			int wid = WEAPON_NONE;
			if (profile_mode == 1)
			{
				int cur = g_Weapon->m_iWeaponID;
				wid = (cur > WEAPON_NONE && cur < WEAPON_MAX_COUNT) ? cur : WEAPON_NONE;
			}
			else if (profile_mode == 2)
				wid = gids[profile_item];
			current_weapon = wid;
			cvars_weapons& W = cvars::weapons[wid];
			float ya = a.y;
			float yb = b.y;
			panel("Weapon Profile", ImVec2(a.x, ya), ImVec2(panel_w, 300.f), [&]()
			{
				const char* wset[] = { "Global", "Current", "Custom" };
				components::combo("Working set", &profile_mode, wset, IM_ARRAYSIZE(wset));
				const char* groups[] = { "Global", "Pistol", "SMG", "Rifle", "Shotgun", "Sniper" };
				components::combo("Group", &weapon_group, groups, IM_ARRAYSIZE(groups));
				if (profile_mode == 2)
					components::combo("Weapon", &profile_item, gnames, gcount);
				if (components::button("Copy to group", ImVec2(-1, 24)))
				{
					for (int i = 0; i < gcount; ++i)
						CopyProfile(wid, gids[i]);
				}
				if (components::button("Copy to all", ImVec2(-1, 24)))
				{
					for (int i = 0; i < WEAPON_MAX_COUNT; ++i)
						CopyProfile(wid, i);
				}
			});
			ya += 308.f;
			panel("Legit Aim", ImVec2(a.x, ya), ImVec2(panel_w, 330.f), [&]()
			{
				components::checkbox("Aim enabled", &W.aim_enabled);
				const char* hb[] = { "Head", "Neck", "Chest", "Stomach", "Arms", "Legs" };
				components::multi_combo("Hitboxes", W.aim_hitboxes, hb, IM_ARRAYSIZE(hb));
				components::slider_float("Aim FOV", &W.aim_fov, 0.f, 30.f, "%.1f", "°");
				components::slider_int("Accuracy boost", &W.aim_accuracy_boost, 0, 5, "%d", "");
			});
			ya += 338.f;
			panel("Recoil", ImVec2(a.x, ya), ImVec2(panel_w, 420.f), [&]()
			{
				components::slider_float("Smooth auto", &W.aim_smooth_auto, 0.f, 30.f, "%.1f", "");
				components::slider_float("Smooth in attack", &W.aim_smooth_in_attack, 0.f, 30.f, "%.1f", "");
				int scf = (int)W.aim_smooth_scale_fov;
				if (components::checkbox("Smooth scale FOV", &scf))
					W.aim_smooth_scale_fov = scf ? 1.f : 0.f;
				components::slider_float("Recoil smooth", &W.aim_recoil_smooth, 0.f, 30.f, "%.1f", "");
				components::slider_float("Recoil FOV", &W.aim_recoil_fov, 0.f, 30.f, "%.1f", "°");
				components::slider_int("Recoil start", &W.aim_recoil_start, 0, 12, "%d", "");
				components::slider_float("Recoil pitch", &W.aim_recoil_pitch, 0.f, 200.f, "%.0f", "%");
				components::slider_float("Recoil yaw", &W.aim_recoil_yaw, 0.f, 200.f, "%.0f", "%");
			});
			ya += 428.f;
			panel("Timings", ImVec2(a.x, ya), ImVec2(panel_w, 270.f), [&]()
			{
				components::slider_int("Lock-on", &W.aim_maximum_lock_on_time, 0, 10000, "%d", "ms");
				components::slider_int("Delay aiming", &W.aim_delay_before_aiming, 0, 1000, "%d", "ms");
				components::slider_int("Delay firing", &W.aim_delay_before_firing, 0, 1000, "%d", "ms");
			});
			panel("pSilent", ImVec2(b.x, yb), ImVec2(panel_w, 360.f), [&]()
			{
				const char* pst[] = { "Off", "In-air", "Silent shots" };
				components::combo("Type", &W.aim_psilent_type, pst, IM_ARRAYSIZE(pst));
				components::slider_float("Angle", &W.aim_psilent_angle, 0.f, 10.f, "%.1f", "°");
				components::checkbox("Tapping mode", &W.aim_psilent_tapping_mode);
				const char* ptr[] = { "Standing", "On land", "In-air" };
				components::multi_combo("Triggers", W.aim_psilent_triggers, ptr, IM_ARRAYSIZE(ptr));
			});
			yb += 368.f;
			panel("Penetration", ImVec2(b.x, yb), ImVec2(panel_w, 170.f), [&]()
			{
				components::checkbox("Auto penetration", &W.aim_auto_penetration);
				components::slider_int("Min damage", &W.aim_auto_penetration_min_damage, 1, 100, "%d", "hp");
			});
			yb += 178.f;
			panel("Automatic", ImVec2(b.x, yb), ImVec2(panel_w, 140.f), [&]()
			{
				components::checkbox("Auto fire", &W.aim_auto_fire);
				components::checkbox("Trigger enabled", &W.trigger_enabled);
			});
			yb += 148.f;
			panel("Trigger", ImVec2(b.x, yb), ImVec2(panel_w, 330.f), [&]()
			{
				const char* thb[] = { "Head", "Neck", "Chest", "Stomach", "Arms", "Legs" };
				components::multi_combo("Hitboxes", W.trigger_hitboxes, thb, IM_ARRAYSIZE(thb));
				components::slider_int("Accuracy boost", &W.trigger_accuracy_boost, 0, 5, "%d", "");
				components::checkbox("Auto penetration", &W.trigger_auto_penetration);
				components::slider_int("Min damage", &W.trigger_auto_penetration_min_damage, 1, 100, "%d", "hp");
			});
			yb += 338.f;
			panel("Rage", ImVec2(b.x, yb), ImVec2(panel_w, 620.f), [&]()
			{
				components::checkbox("Rage enabled", &W.raim_enabled);
				const char* rhb[] = { "Head", "Neck", "Chest", "Stomach", "Arms", "Legs" };
				components::multi_combo("Hitboxes", W.raim_hitboxes, rhb, IM_ARRAYSIZE(rhb));
				components::checkbox("Auto penetration", &W.raim_auto_penetration);
				components::slider_int("Min damage", &W.raim_auto_penetration_min_damage, 1, 100, "%d", "hp");
				components::slider_int("Force damage", &W.raim_force_min_damage, 1, 100, "%d", "hp");
				const char* ast[] = { "Off", "Auto", "Fast", "Early" };
				components::combo("Autostop", &W.raim_autostop, ast, IM_ARRAYSIZE(ast));
				components::checkbox("Autostop crouch", &W.raim_autostop_crouch);
				components::slider_float("Head scale", &W.raim_head_scale, 0.f, 200.f, "%.0f", "%");
				components::text(W.raim_head_scale > 0.5f ? "Head: ON" : "Head: OFF");
				components::slider_float("Neck scale", &W.raim_neck_scale, 0.f, 200.f, "%.0f", "%");
				components::text(W.raim_neck_scale > 0.5f ? "Neck: ON" : "Neck: OFF");
				components::slider_float("Chest scale", &W.raim_chest_scale, 0.f, 200.f, "%.0f", "%");
				components::text(W.raim_chest_scale > 0.5f ? "Chest: ON" : "Chest: OFF");
				components::slider_float("Stomach scale", &W.raim_stomach_scale, 0.f, 200.f, "%.0f", "%");
				components::text(W.raim_stomach_scale > 0.5f ? "Stomach: ON" : "Stomach: OFF");
				components::slider_float("Arms scale", &W.raim_arms_scale, 0.f, 200.f, "%.0f", "%");
				components::text(W.raim_arms_scale > 0.5f ? "Arms: ON" : "Arms: OFF");
				components::slider_float("Legs scale", &W.raim_legs_scale, 0.f, 200.f, "%.0f", "%");
				components::text(W.raim_legs_scale > 0.5f ? "Legs: ON" : "Legs: OFF");
			});
		}
	}
	else if (m_iSelectedTab == ANTIAIM)
	{
		if (section[ANTIAIM] == 0)
		{
			panel("Global AntiAim", a, ImVec2(panel_w, 420.f), [&]()
			{
				const char* roll[] = { "None", "Sideways 50", "Sideways 90", "Sideways 180", "Static" };
				const char* at[] = { "Off", "Yaw to closest", "Yaw to firepower" };
				const char* side[] = { "Off", "Manual left", "Manual right" };
				const char* cond[] = { "On attack", "While nade", "Freeze time" };
				components::checkbox("Enabled", &cvars::ragebot.aa_enabled);
				components::checkbox("Teammates", &cvars::ragebot.aa_teammates);
				components::combo("At targets", &cvars::ragebot.aa_at_targets, at, IM_ARRAYSIZE(at));
				components::multi_combo("Conditions", cvars::ragebot.aa_conditions, cond, IM_ARRAYSIZE(cond));
				components::combo("Side", &cvars::ragebot.aa_side, side, IM_ARRAYSIZE(side));
				components::keybind("Side key", &cvars::ragebot.aa_side_key);
				components::checkbox("Switch when take damage", &cvars::ragebot.aa_side_switch_when_take_damage);
				components::combo("Roll", &cvars::ragebot.aa_roll, roll, IM_ARRAYSIZE(roll));
				components::slider_float("Static roll", &cvars::ragebot.aa_roll_static, -180.f, 180.f, "%.1f", "°");
				components::checkbox("Untrusted checks", &cvars::ragebot.aa_untrusted_checks);
			});
			panel("Yaw Preview", b, ImVec2(panel_w, 420.f), [&]()
			{
				draw_aa_radar(cvars::ragebot.aa_side);
				draw_status_row("Anti-aim enabled", cvars::ragebot.aa_enabled != 0);
				draw_status_row("Roll enabled", cvars::ragebot.aa_roll != 0);
				draw_status_row("Fake lag enabled", cvars::ragebot.fakelag_enabled != 0);
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.68f, 1.f));
				ImGui::TextWrapped("Pink sweep is the real yaw, blue mark is the desync offset.");
				ImGui::PopStyleColor();
			}, true);
		}
		else if (section[ANTIAIM] == 1)
		{
			panel("Stand Angles", a, ImVec2(panel_w, 340.f), [&]()
			{
				const char* pitch[] = { "None", "Down", "Up", "Fake down", "Fake up", "Jitter", "Fake jitter" };
				const char* yaw[] = { "None", "Backwards", "Sideways 90", "Sideways 140", "Static", "Local view", "Desync" };
				components::combo("Pitch", &cvars::ragebot.aa_stand_pitch, pitch, IM_ARRAYSIZE(pitch));
				components::combo("Yaw", &cvars::ragebot.aa_stand_yaw, yaw, IM_ARRAYSIZE(yaw));
				components::slider_float("Static yaw", &cvars::ragebot.aa_stand_yaw_static, -180.f, 180.f, "%.1f", "°");
				components::slider_int("Desync", &cvars::ragebot.aa_stand_desync, 0, 100, "%d", "%");
				components::checkbox("Desync helper", &cvars::ragebot.aa_stand_desync_helper);
			});
			panel("Angle Radar", b, ImVec2(panel_w, 340.f), [&]()
			{
				draw_aa_radar(cvars::ragebot.aa_stand_yaw, 78.f);
				draw_status_row("Anti-aim enabled", cvars::ragebot.aa_enabled != 0);
				draw_status_row("Desync helper", cvars::ragebot.aa_stand_desync_helper != 0);
			}, true);
		}
		else if (section[ANTIAIM] == 2)
		{
			panel("Movement Angles", a, ImVec2(panel_w, 240.f), [&]() 
			{ 
				const char* pitch[] = { "None", "Down", "Up", "Fake down", "Fake up", "Jitter", "Fake jitter" }; 
				const char* yaw[] = { "None", "Backwards", "Local view", "Gait sideways" }; 
				components::combo("Pitch", &cvars::ragebot.aa_move_pitch, pitch, IM_ARRAYSIZE(pitch)); 
				components::combo("Yaw", &cvars::ragebot.aa_move_yaw, yaw, IM_ARRAYSIZE(yaw)); 
			});
			panel("Gait Preview", b, ImVec2(panel_w, 240.f), [&]()
			{
				draw_aa_radar(cvars::ragebot.aa_move_yaw, 56.f);
				draw_status_row("Moving", cvars::kreedz.active != 0);
				draw_status_row("Fake lag", cvars::ragebot.fakelag_enabled != 0);
			}, true);
		}
		else
		{
			panel("Fakelag", a, ImVec2(panel_w, 320.f), [&]()
			{
				const char* type[] = { "Maximum", "Break lag compensation" };
				const char* trig[] = { "Standing", "In air" };
				components::checkbox("Enabled", &cvars::ragebot.fakelag_enabled);
				components::combo("Type", &cvars::ragebot.fakelag_type, type, IM_ARRAYSIZE(type));
				components::slider_int("Choke limit", &cvars::ragebot.fakelag_choke_limit, 1, MAX_TOTAL_CMDS, "%d", "ticks");
				components::multi_combo("Triggers", cvars::ragebot.fakelag_triggers, trig, IM_ARRAYSIZE(trig));
				components::checkbox("While shooting", &cvars::ragebot.fakelag_while_shooting);
				components::checkbox("On enemy in pvs", &cvars::ragebot.fakelag_on_enemy_in_pvs);
				components::checkbox("On peek", &cvars::ragebot.fakelag_on_peek);
			});
			panel("Choke Graph", b, ImVec2(panel_w, 320.f), [&]()
			{
				draw_latency_meter(cvars::ragebot.fakelag_choke_limit * 8);
				ImGui::Spacing();
				draw_tick_bar("Choke limit", cvars::ragebot.fakelag_choke_limit, MAX_TOTAL_CMDS, "ticks");
				ImGui::Spacing();
				draw_status_row("Fake lag", cvars::ragebot.fakelag_enabled != 0);
				draw_status_row("While shooting", cvars::ragebot.fakelag_while_shooting != 0);
				draw_status_row("On peek", cvars::ragebot.fakelag_on_peek != 0);
			}, true);
		}
	}
	else if (m_iSelectedTab == VISUALS)
	{
		if (section[VISUALS] == 0)
		{
			panel("Player ESP", a, ImVec2(panel_w, 900.f), [&]() 
			{ 
				const char* pos[] = { "Off", "Left", "Right", "Top", "Bottom" };
				const char* poslr[] = { "Off", "Left", "Right" };
				const char* box[] = { "Off", "Default", "Corner", "Round" };
				const char* meas[] = { "Units", "Meters" };
				const char* hitb[] = { "Off", "Visible", "All" };
				const char* pl[] = { "Local", "Enemies", "Teammates" };
				components::checkbox("Enabled", &cvars::visuals.active);
				components::keybind("Panic key", &cvars::visuals.panic_key);
				components::slider_float("Dormant time", &cvars::visuals.esp_dormant_time, 0.f, 10.f, "%.1f", "s");
				components::checkbox("Dormant fadeout", &cvars::visuals.esp_dormant_fadeout);
				components::checkbox("Update by sound", &cvars::visuals.esp_dormant_update_by_sound);
				components::checkbox("Interpolate history", &cvars::visuals.esp_interpolate_history);
				components::slider_int("Font", &cvars::visuals.esp_font_size, 8, 24, "%d", "px");
				components::checkbox("Player ESP", &cvars::visuals.esp_player);
				components::multi_combo("Players", cvars::visuals.esp_player_players, pl, IM_ARRAYSIZE(pl));
				components::combo("Box", &cvars::visuals.esp_player_box, box, IM_ARRAYSIZE(box));
				components::slider_int("Box filled", &cvars::visuals.esp_player_box_filled, 0, 100, "%d", "%");
				components::checkbox("Box outline", &cvars::visuals.esp_player_box_outline);
				components::color_edit("Box T", cvars::visuals.esp_player_box_color_t);
				components::color_edit("Box CT", cvars::visuals.esp_player_box_color_ct);
				components::combo("Health", &cvars::visuals.esp_player_health, pos, IM_ARRAYSIZE(pos));
				components::checkbox("Health percentage", &cvars::visuals.esp_player_health_percentage);
				components::color_edit("Health percentage", cvars::visuals.esp_player_health_percentage_color);
				components::combo("Armor", &cvars::visuals.esp_player_armor, poslr, IM_ARRAYSIZE(poslr));
				components::color_edit("Armor", cvars::visuals.esp_player_armor_color);
				components::combo("Name", &cvars::visuals.esp_player_name, pos, IM_ARRAYSIZE(pos));
				components::color_edit("Name", cvars::visuals.esp_player_name_color);
				components::combo("Weapon text", &cvars::visuals.esp_player_weapon_text, pos, IM_ARRAYSIZE(pos));
				components::color_edit("Weapon text", cvars::visuals.esp_player_weapon_text_color);
				components::combo("Weapon icon", &cvars::visuals.esp_player_weapon_icon, pos, IM_ARRAYSIZE(pos));
				components::color_edit("Weapon icon", cvars::visuals.esp_player_weapon_icon_color);
				components::combo("Money", &cvars::visuals.esp_player_money, pos, IM_ARRAYSIZE(pos));
				components::color_edit("Money", cvars::visuals.esp_player_money_color);
				components::combo("Distance", &cvars::visuals.esp_player_distance, pos, IM_ARRAYSIZE(pos));
				components::combo("Measurement", &cvars::visuals.esp_player_distance_measurement, meas, IM_ARRAYSIZE(meas));
				components::color_edit("Distance", cvars::visuals.esp_player_distance_color);
				components::combo("Actions", &cvars::visuals.esp_player_actions, pos, IM_ARRAYSIZE(pos));
				components::checkbox("Actions bar", &cvars::visuals.esp_player_actions_bar);
				components::color_edit("Actions", cvars::visuals.esp_player_actions_color);
				components::color_edit("Actions bar", cvars::visuals.esp_player_actions_bar_color);
				components::combo("Has C4", &cvars::visuals.esp_player_has_c4, pos, IM_ARRAYSIZE(pos));
				components::color_edit("Has C4", cvars::visuals.esp_player_has_c4_color);
				components::combo("Has defusal", &cvars::visuals.esp_player_has_defusal_kits, pos, IM_ARRAYSIZE(pos));
				components::color_edit("Has defusal", cvars::visuals.esp_player_has_defusal_kits_color);
				components::slider_float("Line of sight", &cvars::visuals.esp_player_line_of_sight, 0.f, 500.f, "%.0f", "units");
				components::color_edit("Line of sight", cvars::visuals.esp_player_line_of_sight_color);
				components::combo("Hitboxes", &cvars::visuals.esp_player_hitboxes, hitb, IM_ARRAYSIZE(hitb));
				components::checkbox("Hit position", &cvars::visuals.esp_player_hitboxes_hit_position);
				components::slider_float("Hit position time", &cvars::visuals.esp_player_hitboxes_hit_position_time, 0.f, 5.f, "%.1f", "s");
				components::color_edit("Hit position", cvars::visuals.esp_player_hitboxes_hit_position_color);
				components::color_edit("Hit position 2", cvars::visuals.esp_player_hitboxes_hit_position_color2);
				components::checkbox("Skeleton", &cvars::visuals.esp_player_skeleton);
				components::color_edit("Skeleton", cvars::visuals.esp_player_skeleton_color);
				components::checkbox("Skeleton backtrack", &cvars::visuals.esp_player_skeleton_backtrack);
				components::color_edit("Skeleton backtrack", cvars::visuals.esp_player_skeleton_backtrack_color);
				components::color_edit("Hitboxes", cvars::visuals.esp_player_hitboxes_color);
			});
			panel("Glow", b, ImVec2(panel_w, 300.f), [&]()
			{
				const char* pl[] = { "Local", "Enemies", "Teammates" };
				components::checkbox("Glow", &cvars::visuals.esp_player_glow);
				components::multi_combo("Players", cvars::visuals.esp_player_glow_players, pl, IM_ARRAYSIZE(pl));
				components::slider_int("Amount", &cvars::visuals.esp_player_glow_amount, 0, 255, "%d", "");
				components::checkbox("Health based", &cvars::visuals.esp_player_glow_color_health_based);
				components::color_edit("Glow T", cvars::visuals.esp_player_glow_color_t);
				components::color_edit("Glow CT", cvars::visuals.esp_player_glow_color_ct);
			});
			panel("Sounds", ImVec2(b.x, 310.f), ImVec2(panel_w, 300.f), [&]()
			{
				const char* pl[] = { "Local", "Enemies", "Teammates" };
				components::checkbox("Sounds", &cvars::visuals.esp_player_sounds);
				components::multi_combo("Players", cvars::visuals.esp_player_sounds_players, pl, IM_ARRAYSIZE(pl));
				components::slider_float("Time", &cvars::visuals.esp_player_sounds_time, 0.f, 5.f, "%.1f", "s");
				components::slider_float("Circle radius", &cvars::visuals.esp_player_sounds_circle_radius, 0.f, 100.f, "%.0f", "units");
				components::color_edit("Sounds T", cvars::visuals.esp_player_sounds_color_t);
				components::color_edit("Sounds CT", cvars::visuals.esp_player_sounds_color_ct);
			});
			panel("Out of FOV", ImVec2(b.x, 620.f), ImVec2(panel_w, 380.f), [&]()
			{
				const char* dt[] = { "Arrows", "Circles", "Rhombus" };
				const char* pl[] = { "Enemies", "Teammates" };
				const char* info[] = { "Name", "Weapon", "Distance" };
				components::checkbox("Out of FOV", &cvars::visuals.esp_player_out_of_fov);
				components::combo("Draw type", &cvars::visuals.esp_player_out_of_fov_draw_type, dt, IM_ARRAYSIZE(dt));
				components::multi_combo("Players", cvars::visuals.esp_player_out_of_fov_players, pl, IM_ARRAYSIZE(pl));
				components::multi_combo("Additional info", cvars::visuals.esp_player_out_of_fov_additional_info, info, IM_ARRAYSIZE(info));
				components::slider_float("Size", &cvars::visuals.esp_player_out_of_fov_size, 4.f, 32.f, "%.0f", "px");
				components::slider_float("Radius", &cvars::visuals.esp_player_out_of_fov_radius, 40.f, 300.f, "%.0f", "px");
				components::checkbox("Aspect ratio", &cvars::visuals.esp_player_out_of_fov_aspect_ratio);
				components::checkbox("Impulse alpha", &cvars::visuals.esp_player_out_of_fov_impulse_alpha);
				components::checkbox("Outline", &cvars::visuals.esp_player_out_of_fov_outline);
				components::color_edit("Out of FOV T", cvars::visuals.esp_player_out_of_fov_color_t);
				components::color_edit("Out of FOV CT", cvars::visuals.esp_player_out_of_fov_color_ct);
			});
		}
		else if (section[VISUALS] == 1)
		{
			panel("Dormant", a, ImVec2(panel_w, 200.f), [&]()
			{
				components::slider_float("Dormant time", &cvars::visuals.colored_models_dormant_time, 0.f, 10.f, "%.1f", "s");
				components::checkbox("Fadeout", &cvars::visuals.colored_models_dormant_fadeout);
				components::checkbox("Paint weapons", &cvars::visuals.colored_models_paint_players_weapons);
			});
			panel("Players", ImVec2(a.x, 210.f), ImVec2(panel_w, 520.f), [&]()
			{
				const char* rend[] = { "Off", "Flat", "Darkened", "Lighted", "Textured" };
				const char* hb[] = { "Disabled", "On visible", "Always" };
				const char* pl[] = { "Local", "Enemies", "Teammates" };
				components::combo("Render", &cvars::visuals.colored_models_players, rend, IM_ARRAYSIZE(rend));
				components::multi_combo("Players", cvars::visuals.colored_models_players_players, pl, IM_ARRAYSIZE(pl));
				components::checkbox("Behind wall", &cvars::visuals.colored_models_players_behind_wall);
				components::checkbox("Wireframe", &cvars::visuals.colored_models_players_wireframe);
				components::combo("Health based", &cvars::visuals.colored_models_players_color_health_based, hb, IM_ARRAYSIZE(hb));
				components::color_edit("T hide", cvars::visuals.colored_models_players_color_t_hide);
				components::color_edit("T visible", cvars::visuals.colored_models_players_color_t_vis);
				components::color_edit("CT hide", cvars::visuals.colored_models_players_color_ct_hide);
				components::color_edit("CT visible", cvars::visuals.colored_models_players_color_ct_vis);
				components::combo("On the dead", &cvars::visuals.colored_models_players_on_the_dead, rend, IM_ARRAYSIZE(rend));
				components::color_edit("On the dead", cvars::visuals.colored_models_players_on_the_dead_color);
				components::combo("Hit position", &cvars::visuals.colored_models_players_hit_position, rend, IM_ARRAYSIZE(rend));
				components::slider_float("Hit position time", &cvars::visuals.colored_models_players_hit_position_time, 0.f, 5.f, "%.1f", "s");
				components::color_edit("Hit position", cvars::visuals.colored_models_players_hit_position_color);
				components::combo("Desync AA", &cvars::visuals.colored_models_players_desync_aa, rend, IM_ARRAYSIZE(rend));
				components::color_edit("Desync AA", cvars::visuals.colored_models_players_desync_aa_color);
			});
			panel("Backtrack", ImVec2(b.x, 0.f), ImVec2(panel_w, 300.f), [&]()
			{
				const char* rend[] = { "Off", "Flat", "Darkened", "Lighted", "Textured" };
				const char* pl[] = { "Enemies", "Teammates" };
				components::combo("Render", &cvars::visuals.colored_models_backtrack, rend, IM_ARRAYSIZE(rend));
				components::checkbox("Wireframe", &cvars::visuals.colored_models_backtrack_wireframe);
				components::multi_combo("Players", cvars::visuals.colored_models_backtrack_players, pl, IM_ARRAYSIZE(pl));
				components::checkbox("Behind wall", &cvars::visuals.colored_models_backtrack_behind_wall);
				components::color_edit("T hide", cvars::visuals.colored_models_backtrack_color_t_hide);
				components::color_edit("T visible", cvars::visuals.colored_models_backtrack_color_t_vis);
				components::color_edit("CT hide", cvars::visuals.colored_models_backtrack_color_ct_hide);
				components::color_edit("CT visible", cvars::visuals.colored_models_backtrack_color_ct_vis);
			});
			panel("Hands", ImVec2(b.x, 310.f), ImVec2(panel_w, 220.f), [&]()
			{
				const char* rend[] = { "Off", "Flat", "Darkened", "Lighted", "Textured" };
				components::combo("Render", &cvars::visuals.colored_models_hands, rend, IM_ARRAYSIZE(rend));
				components::checkbox("Wireframe", &cvars::visuals.colored_models_hands_wireframe);
				components::color_edit("Hands", cvars::visuals.colored_models_hands_color);
				components::checkbox("Rainbow", &cvars::visuals.colored_models_hands_color_rainbow);
				components::slider_float("Rainbow speed", &cvars::visuals.colored_models_hands_color_rainbow_speed, 0.1f, 10.f, "%.1f", "");
			});
			panel("Lights", ImVec2(b.x, 540.f), ImVec2(panel_w, 420.f), [&]()
			{
				const char* orig[] = { "Legs", "Body", "Head" };
				const char* pl[] = { "Local", "Enemies", "Teammates" };
				components::checkbox("Dlight", &cvars::visuals.colored_models_dlight);
				components::multi_combo("Dlight players", cvars::visuals.colored_models_dlight_players, pl, IM_ARRAYSIZE(pl));
				components::combo("Dlight origin", &cvars::visuals.colored_models_dlight_origin, orig, IM_ARRAYSIZE(orig));
				components::slider_float("Dlight radius", &cvars::visuals.colored_models_dlight_radius, 0.f, 300.f, "%.0f", "units");
				components::slider_float("Minlight", &cvars::visuals.colored_models_dlight_minlight, 0.f, 100.f, "%.0f", "");
				components::checkbox("Fading lighting", &cvars::visuals.colored_models_dlight_fading_lighting);
				components::slider_float("Fading speed", &cvars::visuals.colored_models_dlight_fading_lighting_speed, 0.1f, 10.f, "%.1f", "");
				components::color_edit("Dlight", cvars::visuals.colored_models_dlight_color);
				components::checkbox("Elight", &cvars::visuals.colored_models_elight);
				components::multi_combo("Elight players", cvars::visuals.colored_models_elight_players, pl, IM_ARRAYSIZE(pl));
				components::slider_float("Elight radius", &cvars::visuals.colored_models_elight_radius, 0.f, 300.f, "%.0f", "units");
				components::color_edit("Elight", cvars::visuals.colored_models_elight_color);
			});
		}
		else if (section[VISUALS] == 2)
		{
			panel("Local ESP", a, ImVec2(panel_w, 660.f), [&]()
			{
				const char* wm[] = { "Time", "Name", "FPS", "Latency", "E4", "E5", "E6", "E7" };
				const char* st[] = { "Hits", "Misses", "Percent" };
				components::checkbox("AA side arrows", &cvars::visuals.esp_other_local_aa_side_arrows);
				components::checkbox("Sniper crosshair", &cvars::visuals.esp_other_local_sniper_crosshair);
				components::checkbox("Recoil point", &cvars::visuals.esp_other_local_recoil_point);
				components::color_edit("Recoil point", cvars::visuals.esp_other_local_recoil_point_color);
				components::checkbox("Spread point", &cvars::visuals.esp_other_local_spread_point);
				components::color_edit("Spread point", cvars::visuals.esp_other_local_spread_point_color);
				components::checkbox("Spread circle", &cvars::visuals.esp_other_local_spread_circle);
				components::color_edit("Spread circle", cvars::visuals.esp_other_local_spread_circle_color);
				components::color_edit("Spread circle 2", cvars::visuals.esp_other_local_spread_circle_color2);
				components::checkbox("Aim FOV", &cvars::visuals.esp_other_local_aim_fov);
				components::color_edit("Aim FOV", cvars::visuals.esp_other_local_aim_fov_color);
				components::color_edit("Aim FOV 2", cvars::visuals.esp_other_local_aim_fov_color2);
				components::checkbox("Toggle status", &cvars::visuals.esp_other_local_toggle_status);
				components::color_edit("Toggle status", cvars::visuals.esp_other_local_toggle_status_color);
				components::color_edit("Toggle status 2", cvars::visuals.esp_other_local_toggle_status_color2);
				components::checkbox("Hud clear", &cvars::visuals.effects_hud_clear);
				components::slider_int("Custom render FOV", &cvars::visuals.effects_custom_render_fov, 10, 150, "%d", "°");
				components::checkbox("Watermark credits", &cvars::visuals.watermark_credits);
				components::multi_combo("Watermark elements", cvars::visuals.watermark_elements, wm, IM_ARRAYSIZE(wm));
				components::checkbox("Crosshair", &cvars::visuals.crosshair_enabled);
				components::checkbox("Crosshair dot", &cvars::visuals.crosshair_dot);
				components::slider_int("Crosshair size", &cvars::visuals.crosshair_size, 1, 40, "%d", "px");
				components::slider_int("Crosshair gap", &cvars::visuals.crosshair_gap, 0, 40, "%d", "px");
				components::slider_int("Crosshair thickness", &cvars::visuals.crosshair_thickness, 1, 10, "%d", "px");
				components::checkbox("Dynamic crosshair", &cvars::visuals.crosshair_dynamic);
				components::slider_int("Dynamic scale", &cvars::visuals.crosshair_dynamic_scale, 0, 100, "%d", "%");
				components::checkbox("Function status", &cvars::visuals.local_function_status);
				components::checkbox("Statistics", &cvars::visuals.local_statistics);
				components::multi_combo("Statistics elements", cvars::visuals.local_statistics_elements, st, IM_ARRAYSIZE(st));
				components::checkbox("Bullet tracers", &cvars::visuals.local_bullet_tracers);
				components::slider_float("Tracers time", &cvars::visuals.local_bullet_tracers_time, 0.1f, 5.f, "%.1f", "s");
				components::color_edit("Tracers", cvars::visuals.local_bullet_tracers_color);
				components::checkbox("Thirdperson", &cvars::visuals.effects_thirdperson, &cvars::visuals.effects_thirdperson_key);
				components::keybind("Thirdperson key", &cvars::visuals.effects_thirdperson_key);
				components::slider_int("Thirdperson distance", &cvars::visuals.effects_thirdperson, 0, 10, "%d", "units");
				components::checkbox("Hotkey list", &cvars::visuals.local_hotkey_list);
			});
		}
		else
		{
			panel("World", a, ImVec2(panel_w, 400.f), [&]()
			{
				components::checkbox("Remove smoke", &cvars::visuals.remove_smoke);
				components::checkbox("Remove scope", &cvars::visuals.remove_scope);
				components::checkbox("Remove recoil", &cvars::visuals.remove_visual_recoil);
				components::checkbox("Watermark", &cvars::visuals.watermark);
				components::checkbox("Notifications", &cvars::visuals.main_notifications);
				components::checkbox("Stream mode", &cvars::visuals.streamer_mode);
				components::checkbox("Antiscreen", &cvars::visuals.antiscreen);
				components::checkbox("Copyright", &cvars::visuals.copyright);
				if (components::slider_float("Copyright X", &cvars::visuals.copyright_position[0], 0.f, 1920.f, "%.0f", "px"))
					cvars::visuals.copyright_position[2] = 0.f;
				if (components::slider_float("Copyright Y", &cvars::visuals.copyright_position[1], 0.f, 1080.f, "%.0f", "px"))
					cvars::visuals.copyright_position[2] = 0.f;
				components::keybind("Panic key", &cvars::visuals.panic_key);
components::checkbox("Allow resize", &cvars::visuals.main_allow_resize); 
			});
		}
	}
	else if (m_iSelectedTab == MOVEMENT)
	{
		if (section[MOVEMENT] == 0)
		{
			panel("Main Movement", a, ImVec2(panel_w, 470.f), [&]() 
			{ 
				components::checkbox("Enabled", &cvars::kreedz.active, &cvars::kreedz.key); 
				components::keybind("Key", &cvars::kreedz.key); 
				components::checkbox("Bunnyhop", &cvars::kreedz.bunnyhop); 
				components::checkbox("Ground strafe", &cvars::kreedz.groundstrafe); 
				components::slider_float("Ground gain", &cvars::kreedz.groundstrafe_gain, 0.f, 1.f, "%.2f"); 
				components::checkbox("Fast run", &cvars::kreedz.fastrun, &cvars::kreedz.fastrun_key); 
				components::keybind("Fast run key", &cvars::kreedz.fastrun_key); 
				components::checkbox("Slow walk", &cvars::kreedz.slowwalk, &cvars::kreedz.slowwalk_key); 
				components::keybind("Slow walk key", &cvars::kreedz.slowwalk_key); 
				components::slider_float("Slow walk speed", &cvars::kreedz.slowwalk_speed, 1.f, 320.f, "%.0f", "u/s"); 
			});
			panel("Movement Status", b, ImVec2(panel_w, 470.f), [&]()
			{
				draw_tick_bar("Slow walk speed", static_cast<int>(cvars::kreedz.slowwalk_speed), 320, "u/s");
				ImGui::Spacing();
				draw_tick_bar("Ground gain", static_cast<int>(cvars::kreedz.groundstrafe_gain * 100.f), 100, "%");
				ImGui::Spacing();
				draw_status_row("Enabled", cvars::kreedz.active != 0);
				draw_status_row("Bunnyhop", cvars::kreedz.bunnyhop != 0);
				draw_status_row("Ground strafe", cvars::kreedz.groundstrafe != 0);
				draw_status_row("Fast run", cvars::kreedz.fastrun != 0);
				draw_status_row("Slow walk", cvars::kreedz.slowwalk != 0);
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.68f, 1.f));
				ImGui::TextWrapped("Bars reflect the current speed limits applied to your movement.");
				ImGui::PopStyleColor();
			}, true);
		}
		else if (section[MOVEMENT] == 1)
		{
			panel("Strafes & Helpers", a, ImVec2(panel_w, 420.f), [&]() 
			{ 
				components::checkbox("Rage strafe", &cvars::kreedz.rage_strafe); 
				components::checkbox("Legit strafe", &cvars::kreedz.legit_strafe); 
				components::slider_float("Legit speed", &cvars::kreedz.legit_strafe_speed, 1.f, 100.f, "%.1f", "%"); 
				components::checkbox("DeGen", &cvars::kreedz.degen, &cvars::kreedz.degen_key); 
				components::keybind("DeGen key", &cvars::kreedz.degen_key); 
				components::slider_float("DeGen power", &cvars::kreedz.degen_power, 50.f, 1000.f, "%.0f"); 
components::slider_float("DeGen tilt", &cvars::kreedz.degen_tilt, -0.9f, 0.9f, "%.2f"); 
			});
			panel("Strafe Preview", b, ImVec2(panel_w, 420.f), [&]()
			{
				draw_tick_bar("Legit speed", static_cast<int>(cvars::kreedz.legit_strafe_speed), 100, "%");
				ImGui::Spacing();
				draw_tick_bar("DeGen power", static_cast<int>(cvars::kreedz.degen_power), 1000, "u/s");
				ImGui::Spacing();
				draw_status_row("Rage strafe", cvars::kreedz.rage_strafe != 0);
				draw_status_row("Legit strafe", cvars::kreedz.legit_strafe != 0);
				draw_status_row("DeGen", cvars::kreedz.degen != 0);
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.68f, 1.f));
				ImGui::TextWrapped("Tune the gain meters until both bars read comfortably below the limit.");
				ImGui::PopStyleColor();
			}, true);
		}
		else
		{
			panel("Movement Exploits", a, ImVec2(panel_w, 310.f), [&]() 
			{ 
				components::checkbox("Jump bug", &cvars::kreedz.jumpbug); 
				components::checkbox("Edge bug", &cvars::kreedz.edgebug); 
				components::checkbox("Wall bug", &cvars::kreedz.wallbug); 
				components::checkbox("Auto jump of fall", &cvars::kreedz.auto_jof); 
				components::slider_float("JOF distance", &cvars::kreedz.auto_jof_min_distance, 1.f, 16.f, "%.1f", "units"); 
			});
			panel("Bug Telemetry", b, ImVec2(panel_w, 310.f), [&]()
			{
				draw_tick_bar("JOF distance", static_cast<int>(cvars::kreedz.auto_jof_min_distance), 16, "units");
				ImGui::Spacing();
				draw_status_row("Jump bug", cvars::kreedz.jumpbug != 0);
				draw_status_row("Edge bug", cvars::kreedz.edgebug != 0);
				draw_status_row("Wall bug", cvars::kreedz.wallbug != 0);
				draw_status_row("Auto jump off fall", cvars::kreedz.auto_jof != 0);
			}, true);
		}
	}
	else if (m_iSelectedTab == MISC)
	{
		if (section[MISC] == 0)
		{
			panel("Exploits", a, ImVec2(panel_w, 640.f), [&]() 
			{ 
				components::checkbox("Doubletap", &cvars::misc.doubletap, &cvars::misc.doubletap_key); 
				components::keybind("Doubletap key", &cvars::misc.doubletap_key); 
				components::slider_int("Doubletap shift", &cvars::misc.doubletap_shift, 1, 4, "%d", "ticks"); 
				components::checkbox("Airstuck", &cvars::misc.airstuck, &cvars::misc.airstuck_key); 
				components::keybind("Airstuck key", &cvars::misc.airstuck_key); 
				components::checkbox("Fakelatency", &cvars::misc.fakelatency); 
				components::slider_int("Latency", &cvars::misc.fakelatency_amount, 0, 500, "%d", "ms"); 
components::keybind("Fakelatency key", &cvars::misc.fakelatency_key); 
				components::checkbox("Hold key", &cvars::misc.fakelatency_hold); 
				components::checkbox("Name stealer", &cvars::misc.namestealer); 
				components::slider_int("Name interval", &cvars::misc.namestealer_interval, 1, 3600, "%d", "s"); 
				components::checkbox("Predict local", &cvars::misc.predict_local); 
				components::checkbox("Predict players", &cvars::misc.predict_players);
			});
			panel("Telemetry", b, ImVec2(panel_w, 640.f), [&]()
			{
				draw_latency_meter(cvars::misc.fakelatency_amount);
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.68f, 1.f));
				ImGui::TextUnformatted("Fake latency load");
				ImGui::PopStyleColor();
				ImGui::Spacing();
				draw_tick_bar("Doubletap shift", cvars::misc.doubletap_shift, 4, "ticks");
				draw_tick_bar("Name interval", cvars::misc.namestealer_interval, 60, "s");
				ImGui::Spacing();
				draw_status_row("Doubletap", cvars::misc.doubletap != 0);
				draw_status_row("Airstuck", cvars::misc.airstuck != 0);
				draw_status_row("Fake latency", cvars::misc.fakelatency != 0);
				draw_status_row("Name stealer", cvars::misc.namestealer != 0);
				draw_status_row("Predict local", cvars::misc.predict_local != 0);
				draw_status_row("Predict players", cvars::misc.predict_players != 0);
			}, true);
		}
		else if (section[MISC] == 1)
		{
			panel("Automation", a, ImVec2(panel_w, 560.f), [&]() 
			{ 
				components::checkbox("Automatic reload", &cvars::misc.automatic_reload); 
				components::checkbox("Automatic pistol", &cvars::misc.automatic_pistol); 
				components::checkbox("No spread", &cvars::misc.nospread);
				const char* nsm[] = { "Off", "Compensate", "Perfect" };
				components::combo("Spread mode", &cvars::misc.nospread_mode, nsm, IM_ARRAYSIZE(nsm)); 
				components::checkbox("Unlock FPS", &cvars::misc.fps_unlock); 
				components::checkbox("Frame skip", &cvars::misc.frame_skip); 
				components::slider_int("Skip amount", &cvars::misc.frame_skip_amount, 1, 10, "%d", "frames"); 
components::checkbox("Developer", &cvars::misc.fps_developer); 
				components::checkbox("Client weapons", &cvars::misc.client_weapons); 
				components::checkbox("Replace models", &cvars::misc.replace_models_with_original); 
				components::checkbox("Maximize on respawn", &cvars::misc.maximize_on_respawn);
			});
			panel("Performance", b, ImVec2(panel_w, 560.f), [&]()
			{
				draw_latency_meter(cvars::misc.frame_skip_amount * 40);
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.68f, 1.f));
				ImGui::TextUnformatted("Frame timing load");
				ImGui::PopStyleColor();
				ImGui::Spacing();
				draw_tick_bar("Frame skip", cvars::misc.frame_skip_amount, 10, "frames");
				ImGui::Spacing();
				draw_status_row("Automatic reload", cvars::misc.automatic_reload != 0);
				draw_status_row("Automatic pistol", cvars::misc.automatic_pistol != 0);
				draw_status_row("No spread", cvars::misc.nospread != 0);
				draw_status_row("Unlock FPS", cvars::misc.fps_unlock != 0);
				draw_status_row("Frame skip", cvars::misc.frame_skip != 0);
				draw_status_row("Client weapons", cvars::misc.client_weapons != 0);
			}, true);
		}
		else if (section[MISC] == 2)
		{
			panel("Knifebot", a, ImVec2(panel_w, 700.f), [&]() 
			{ 
				components::checkbox("Enabled", &cvars::misc.kb_enabled, &cvars::misc.kb_key); 
				components::keybind("Key", &cvars::misc.kb_key); 
				components::checkbox("Friendly fire", &cvars::misc.kb_friendly_fire); 
				components::slider_float("Maximum FOV", &cvars::misc.kb_fov, 0.f, 180.f, "%.1f", "°"); 
				components::slider_float("Swing distance", &cvars::misc.kb_swing_distance, 16.f, 64.f, "%.1f", "units"); 
				components::slider_float("Stab distance", &cvars::misc.kb_stab_distance, 16.f, 64.f, "%.1f", "units"); 
const char* kat[] = { "Swing", "Stab" }; 
				components::combo("Attack type", &cvars::misc.kb_attack_type, kat, IM_ARRAYSIZE(kat)); 
				const char* kaim[] = { "Off", "View angles", "Silent", "Teleport" }; 
				components::combo("Aim type", &cvars::misc.kb_aim_type, kaim, IM_ARRAYSIZE(kaim)); 
				const char* khb[] = { "Head", "Neck", "Chest", "Stomach", "Arms", "Legs" }; 
				components::multi_combo("Aim hitboxes", cvars::misc.kb_aim_hitbox, khb, IM_ARRAYSIZE(khb)); 
				components::slider_float("Hitbox scale", &cvars::misc.kb_aim_hitbox_scale, 20.f, 200.f, "%.0f", "%"); 
				components::checkbox("Position adjustment", &cvars::misc.kb_position_adjustment); 
				const char* kcond[] = { "No spectators", "No back attacks", "No shield" }; 
				components::multi_combo("Conditions", cvars::misc.kb_conditions, kcond, IM_ARRAYSIZE(kcond)); 
				components::checkbox("Debug", &cvars::misc.kb_debug);
			});
			panel("Knife Range", b, ImVec2(panel_w, 700.f), [&]()
			{
				draw_tick_bar("Maximum FOV", static_cast<int>(cvars::misc.kb_fov), 180, "deg");
				ImGui::Spacing();
				draw_tick_bar("Swing distance", static_cast<int>(cvars::misc.kb_swing_distance), 64, "units");
				ImGui::Spacing();
				draw_tick_bar("Stab distance", static_cast<int>(cvars::misc.kb_stab_distance), 64, "units");
				ImGui::Spacing();
				draw_tick_bar("Hitbox scale", static_cast<int>(cvars::misc.kb_aim_hitbox_scale), 200, "%");
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.68f, 1.f));
				ImGui::TextUnformatted("Conditions");
				ImGui::PopStyleColor();
				ImGui::Spacing();
				draw_status_row("Knifebot", cvars::misc.kb_enabled != 0);
				draw_status_row("Friendly fire", cvars::misc.kb_friendly_fire != 0);
				draw_status_row("Position adjustment", cvars::misc.kb_position_adjustment != 0);
				draw_status_row("Debug overlay", cvars::misc.kb_debug != 0);
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.68f, 1.f));
				ImGui::TextWrapped("Swing and stab bars must stay above the target distance or the bot will whiff.");
				ImGui::PopStyleColor();
			}, true);
		}
		else if (section[MISC] == 3)
		{
			panel("Other Features", a, ImVec2(panel_w, 480.f), [&]() 
			{ 
				components::checkbox("Auto block", &cvars::misc.auto_block, &cvars::misc.auto_block_key); 
				components::keybind("Auto block key", &cvars::misc.auto_block_key); 
				components::checkbox("Auto boost", &cvars::misc.auto_boost); 
				components::checkbox("Block MOTD", &cvars::misc.motd_block); 
				components::checkbox("Sandbox mode", &cvars::misc.sandbox_enabled); 
				components::checkbox("SteamID spoofer", &cvars::misc.steamid_spoofer); 
components::keybind("Auto boost key", &cvars::misc.auto_boost_key); 
				components::checkbox("Game speed", &cvars::misc.speed_enabled, &cvars::misc.speed_key); 
				components::keybind("Speed key", &cvars::misc.speed_key); 
				components::slider_float("Speed value", &cvars::misc.speed_value, 0.1f, 5.f, "%.1f", "x");
			});
			panel("Feature Status", b, ImVec2(panel_w, 480.f), [&]()
			{
				draw_tick_bar("Game speed", static_cast<int>(cvars::misc.speed_value * 20.f), 100, "%");
				ImGui::Spacing();
				draw_status_row("Auto block", cvars::misc.auto_block != 0);
				draw_status_row("Auto boost", cvars::misc.auto_boost != 0);
				draw_status_row("Block MOTD", cvars::misc.motd_block != 0);
				draw_status_row("Sandbox mode", cvars::misc.sandbox_enabled != 0);
				draw_status_row("SteamID spoofer", cvars::misc.steamid_spoofer != 0);
				draw_status_row("Game speed", cvars::misc.speed_enabled != 0);
			}, true);
		}
		else
		{
			panel("Theme & Colors", a, ImVec2(panel_w, 280.f), [&]()
			{
				static float col[4] = { accent.x, accent.y, accent.z, accent.w };
				if (components::color_edit("Accent Color", col))
				{
					components::set_accent_color(ImVec4(col[0], col[1], col[2], col[3]));
				}
				ImGui::Spacing();
				if (components::button("Pink / Rose (Default)", ImVec2(-1, 24)))
				{
					components::set_accent_color(ImVec4(1.0f, 0.30f, 0.62f, 1.f));
					col[0] = 1.0f; col[1] = 0.30f; col[2] = 0.62f; col[3] = 1.f;
				}
				if (components::button("Purple / Violet", ImVec2(-1, 24)))
				{
					components::set_accent_color(ImVec4(0.75f, 0.35f, 0.95f, 1.f));
					col[0] = 0.75f; col[1] = 0.35f; col[2] = 0.95f; col[3] = 1.f;
				}
				if (components::button("Blue / Cyan", ImVec2(-1, 24)))
				{
					components::set_accent_color(ImVec4(0.20f, 0.65f, 0.95f, 1.f));
					col[0] = 0.20f; col[1] = 0.65f; col[2] = 0.95f; col[3] = 1.f;
				}
				if (components::button("Green / Skeet", ImVec2(-1, 24)))
				{
					components::set_accent_color(ImVec4(0.40f, 0.85f, 0.30f, 1.f));
					col[0] = 0.40f; col[1] = 0.85f; col[2] = 0.30f; col[3] = 1.f;
				}
			});
			panel("Accent Preview", b, ImVec2(panel_w, 280.f), [&]()
			{
				static const ImVec4 presets[] =
				{
					ImVec4(1.00f, 0.30f, 0.62f, 1.f), ImVec4(0.75f, 0.35f, 0.95f, 1.f),
					ImVec4(0.20f, 0.65f, 0.95f, 1.f), ImVec4(0.40f, 0.85f, 0.30f, 1.f),
					ImVec4(0.95f, 0.75f, 0.25f, 1.f), ImVec4(0.95f, 0.30f, 0.30f, 1.f),
				};
				ImDrawList* dl = ImGui::GetWindowDrawList();
				ImVec2 cur = ImGui::GetCursorScreenPos();
				const float avail_w = ImGui::GetContentRegionAvail().x;
				const float sw = (avail_w - 10.f) / 3.f;
				for (int i = 0; i < 6; ++i)
				{
					const int cx = i % 3;
					const int cy = i / 3;
					const ImVec2 c0 = cur + ImVec2(static_cast<float>(cx) * (sw + 5.f), static_cast<float>(cy) * 40.f);
					const ImVec2 c1 = c0 + ImVec2(sw, 34.f);
					const float pulse = 0.5f + 0.5f * ImSin(static_cast<float>(ImGui::GetTime()) * 1.6f + static_cast<float>(i));
					dl->AddRectFilled(c0 - ImVec2(2.f, 2.f), c1 + ImVec2(2.f, 2.f), ImColor(presets[i].x, presets[i].y, presets[i].z, 0.10f + 0.18f * pulse), 9.f);
					dl->AddRectFilled(c0, c1, ImColor(presets[i]), 7.f);
					dl->AddRect(c0, c1, ImColor(1.f, 1.f, 1.f, 0.12f), 7.f);
				}
				ImGui::Dummy(ImVec2(avail_w, 84.f));
				ImGui::Spacing();
				draw_status_row("Neon accent active", true);
				ImGui::Spacing();
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.60f, 0.60f, 0.68f, 1.f));
				ImGui::TextWrapped("Swatches preview the available accents. Pick one on the left to apply it instantly.");
				ImGui::PopStyleColor();
			}, true);
		}
	}
		else if (m_iSelectedTab == CONFIGS)
		{
			if (section[CONFIGS] == 0)
			{
				panel("Configurations", a, ImVec2(content_w, 410.f), [&]() 
				{ 
					m_pSettingsList->RefreshSettingsList(); 
					m_pGui->ListBox("##settings", &m_pSettingsList->m_selected_settings, m_pSettingsList->m_settings_list, 10); 
					if (components::button("Load", ImVec2(196.f, 28.f))) g_pSettings->Load(m_pSettingsList->GetFocusSettings()); 
					ImGui::SameLine();
					if (components::button("Save", ImVec2(196.f, 28.f))) g_pSettings->Save(m_pSettingsList->GetFocusSettings()); 
				});
			}
			else
			{
				panel("Binds", a, ImVec2(content_w, ImMin(content_h, 520.f)), [&]()
				{
					for (size_t i = 0; i < IM_ARRAYSIZE(g_bind_rows); ++i)
					{
						ImGui::PushID(static_cast<int>(i));
						components::keybind(g_bind_rows[i].label, g_bind_rows[i].bind);
						ImGui::PopID();
					}
				});
			}
		}
	else
	{
		g_Console.Draw("##console", nullptr);
	}

	ImGui::EndChild();
	if (g_pPrimTextFont)
		ImGui::PopFont();
	ImGui::PopStyleColor(8);
	ImGui::PopStyleVar(3);
	m_pGui->End();
}

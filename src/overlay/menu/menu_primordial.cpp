#include "framework.h"
#include "overlay/font_atlas.h"
#include "overlay/menu/gui/components.h"

namespace
{
	void panel(const char* name, const ImVec2& pos, const ImVec2& size, const std::function<void()>& draw)
	{
		ImGui::SetCursorPos(pos);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.105f, 0.105f, 0.112f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.18f, 0.19f, 1.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.f, 10.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.f);
		ImGui::BeginChild(name, size, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.91f, 0.91f, 0.92f, 1.f));
		ImGui::TextUnformatted(name);
		ImGui::PopStyleColor();
		ImGui::Separator();
		draw();
		ImGui::EndChild();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(2);
	}

	bool side(const char* label, bool active)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, active ? ImVec4(0.18f, 0.18f, 0.19f, 1.f) : ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.18f, 0.19f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.23f, 0.23f, 0.24f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_Text, active ? ImVec4(0.94f, 0.94f, 0.95f, 1.f) : ImVec4(0.62f, 0.62f, 0.64f, 1.f));
		const bool hit = ImGui::Button(label, ImVec2(134.f, 32.f));
		const ImVec2 min = ImGui::GetItemRectMin();
		if (active)
			GImGui->CurrentWindow->DrawList->AddRectFilled(min, min + ImVec2(2.f, 32.f), ImGui::GetColorU32(ImGuiCol_CheckMark));
		ImGui::PopStyleColor(4);
		return hit;
	}

	void tabs(const std::vector<const char*>& labels, int& selected)
	{
		for (size_t i = 0; i < labels.size(); ++i)
		{
			if (side(labels[i], selected == static_cast<int>(i)))
				selected = static_cast<int>(i);
		}
	}
}

void CMenu::Draw()
{
	enum { AIMBOT, ANTIAIM, VISUALS, MOVEMENT, MISC, CONFIGS, CONSOLE };
	static int section[7]{};
	static int weapon_group{};
	static int current_weapon = WEAPON_NONE;
	static bool current_weapon_auto{};
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

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 4.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7.f, 4.f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.82f, 0.82f, 0.84f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.43f, 0.43f, 0.45f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.16f, 0.17f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.21f, 0.21f, 0.22f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.24f, 0.24f, 0.25f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, accent);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, accent);
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(accent.x * 1.15f, accent.y * 1.15f, accent.z * 1.15f, 1.f));
	
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
			panel("Primary", a, ImVec2(panel_w, 330.f), [&]()
			{
				components::checkbox("Enabled", &cvars::legitbot.active, &cvars::legitbot.aim_key);
				components::keybind("Aim key", &cvars::legitbot.aim_key);
				components::checkbox("Friendly fire", &cvars::legitbot.friendly_fire);
				components::checkbox("Automatic scope", &cvars::legitbot.aim_auto_scope);
				components::checkbox("Smoke check", &cvars::legitbot.aim_smoke_check);
				components::checkbox("Dynamic FOV", &cvars::legitbot.aim_dynamic_fov);
			});
			panel("Target Weights", b, ImVec2(panel_w, 380.f), [&]()
			{
				components::slider_int("Head weight", &cvars::legitbot.target_weight_head, 1, 100, "%d", "%");
				components::slider_int("Chest weight", &cvars::legitbot.target_weight_chest, 1, 100, "%d", "%");
				components::slider_int("Stomach weight", &cvars::legitbot.target_weight_stomach, 1, 100, "%d", "%");
				components::slider_int("Arms weight", &cvars::legitbot.target_weight_arms, 1, 100, "%d", "%");
				components::slider_int("Legs weight", &cvars::legitbot.target_weight_legs, 1, 100, "%d", "%");
			});
		}
		else if (section[AIMBOT] == 1)
		{
			panel("Ragebot", a, ImVec2(panel_w, 300.f), [&]()
			{
				components::checkbox("Enabled", &cvars::ragebot.active, &cvars::ragebot.raim_key);
				components::keybind("Aim key", &cvars::ragebot.raim_key);
				components::checkbox("Friendly fire", &cvars::ragebot.friendly_fire);
				components::checkbox("Automatic fire", &cvars::ragebot.raim_auto_fire);
				components::slider_float("Maximum FOV", &cvars::ragebot.raim_fov, 0.f, 180.f, "%.1f", "°");
			});
			panel("Weapon Config", b, ImVec2(panel_w, 280.f), [&]()
			{
				components::checkbox("Enabled", &cvars::weapons[WEAPON_NONE].raim_enabled);
				components::checkbox("Automatic penetration", &cvars::weapons[WEAPON_NONE].raim_auto_penetration);
				components::slider_int("Minimum damage", &cvars::weapons[WEAPON_NONE].raim_auto_penetration_min_damage, 1, 100, "%d", "hp");
			});
		}
		else if (section[AIMBOT] == 2)
		{
			panel("Triggerbot", a, ImVec2(ImMin(content_w, 420.f), 310.f), [&]()
			{
				components::keybind("Trigger key", &cvars::legitbot.trigger_key);
				components::checkbox("Accurate traces", &cvars::legitbot.trigger_accurate_traces);
				components::checkbox("Only scoped", &cvars::legitbot.trigger_only_scoped);
				components::checkbox("Dynamic", &cvars::legitbot.trigger_dynamic);
				components::slider_int("Shot delay", &cvars::legitbot.trigger_shot_delay, 0, 1000, "%d", "ms");
			});
		}
		else
		{
			panel("Weapon Profile", a, ImVec2(content_w, 280.f), [&]()
			{
				components::checkbox("Current weapon", &current_weapon_auto);
				const char* groups[] = { "Global", "Pistol", "SMG", "Rifle", "Shotgun", "Sniper" };
				components::combo("Group", &weapon_group, groups, IM_ARRAYSIZE(groups));
				current_weapon = WEAPON_NONE;
				components::checkbox("Aim enabled", &cvars::weapons[current_weapon].aim_enabled);
				components::slider_float("Aim FOV", &cvars::weapons[current_weapon].aim_fov, 0.f, 180.f, "%.1f", "°");
			});
		}
	}
	else if (m_iSelectedTab == ANTIAIM)
	{
		if (section[ANTIAIM] == 0)
		{
			panel("Global AntiAim", a, ImVec2(ImMin(content_w, 420.f), 340.f), [&]() 
			{ 
				const char* roll[] = { "None", "Sideways 50", "Sideways 90", "Sideways 180", "Static" }; 
				components::checkbox("Enabled", &cvars::ragebot.aa_enabled); 
				components::checkbox("Teammates", &cvars::ragebot.aa_teammates); 
				components::combo("Roll", &cvars::ragebot.aa_roll, roll, IM_ARRAYSIZE(roll)); 
				components::slider_float("Static roll", &cvars::ragebot.aa_roll_static, -180.f, 180.f, "%.1f", "°"); 
				components::checkbox("Untrusted checks", &cvars::ragebot.aa_untrusted_checks); 
			});
		}
		else if (section[ANTIAIM] == 1)
		{
			panel("Stand Angles", a, ImVec2(ImMin(content_w, 420.f), 340.f), [&]() 
			{ 
				const char* pitch[] = { "None", "Down", "Up", "Fake down", "Fake up", "Jitter", "Fake jitter" }; 
				const char* yaw[] = { "None", "Backwards", "Sideways 90", "Sideways 140", "Static", "Local view", "Desync" }; 
				components::combo("Pitch", &cvars::ragebot.aa_stand_pitch, pitch, IM_ARRAYSIZE(pitch)); 
				components::combo("Yaw", &cvars::ragebot.aa_stand_yaw, yaw, IM_ARRAYSIZE(yaw)); 
				components::slider_float("Static yaw", &cvars::ragebot.aa_stand_yaw_static, -180.f, 180.f, "%.1f", "°"); 
				components::checkbox("Desync helper", &cvars::ragebot.aa_stand_desync_helper); 
			});
		}
		else if (section[ANTIAIM] == 2)
		{
			panel("Movement Angles", a, ImVec2(ImMin(content_w, 420.f), 240.f), [&]() 
			{ 
				const char* pitch[] = { "None", "Down", "Up", "Fake down", "Fake up", "Jitter", "Fake jitter" }; 
				const char* yaw[] = { "None", "Backwards", "Local view", "Gait sideways" }; 
				components::combo("Pitch", &cvars::ragebot.aa_move_pitch, pitch, IM_ARRAYSIZE(pitch)); 
				components::combo("Yaw", &cvars::ragebot.aa_move_yaw, yaw, IM_ARRAYSIZE(yaw)); 
			});
		}
		else
		{
			panel("Fakelag", a, ImVec2(ImMin(content_w, 420.f), 320.f), [&]() 
			{ 
				const char* type[] = { "Maximum", "Break lag compensation" }; 
				components::checkbox("Enabled", &cvars::ragebot.fakelag_enabled); 
				components::combo("Type", &cvars::ragebot.fakelag_type, type, IM_ARRAYSIZE(type)); 
				components::slider_int("Choke limit", &cvars::ragebot.fakelag_choke_limit, 1, MAX_TOTAL_CMDS, "%d", "ticks"); 
				components::checkbox("While shooting", &cvars::ragebot.fakelag_while_shooting); 
				components::checkbox("On peek", &cvars::ragebot.fakelag_on_peek); 
			});
		}
	}
	else if (m_iSelectedTab == VISUALS)
	{
		if (section[VISUALS] == 0)
		{
			panel("Player ESP", a, ImVec2(ImMin(content_w, 420.f), 430.f), [&]() 
			{ 
				const char* pos[] = { "Off", "Left", "Right", "Top", "Bottom" }; 
				components::checkbox("Enabled", &cvars::visuals.active); 
				components::checkbox("Player ESP", &cvars::visuals.esp_player); 
				components::combo("Box", &cvars::visuals.esp_player_box, pos, IM_ARRAYSIZE(pos)); 
				components::checkbox("Box outline", &cvars::visuals.esp_player_box_outline); 
				components::combo("Health", &cvars::visuals.esp_player_health, pos, IM_ARRAYSIZE(pos)); 
				components::combo("Name", &cvars::visuals.esp_player_name, pos, IM_ARRAYSIZE(pos)); 
				components::combo("Weapon text", &cvars::visuals.esp_player_weapon_text, pos, IM_ARRAYSIZE(pos)); 
				components::checkbox("Skeleton", &cvars::visuals.esp_player_skeleton); 
			});
		}
		else if (section[VISUALS] == 1)
		{
			panel("Chams & Models", a, ImVec2(ImMin(content_w, 420.f), 300.f), [&]() 
			{ 
				components::checkbox("Players", &cvars::visuals.colored_models_players); 
				components::checkbox("Behind wall", &cvars::visuals.colored_models_players_behind_wall); 
				components::checkbox("Wireframe", &cvars::visuals.colored_models_players_wireframe); 
				components::checkbox("Hands", &cvars::visuals.colored_models_hands); 
				components::checkbox("Backtrack", &cvars::visuals.colored_models_backtrack); 
			});
		}
		else if (section[VISUALS] == 2)
		{
			panel("Local Visuals", a, ImVec2(ImMin(content_w, 420.f), 340.f), [&]() 
			{ 
				components::checkbox("Crosshair", &cvars::visuals.crosshair_enabled); 
				components::checkbox("Dynamic crosshair", &cvars::visuals.crosshair_dynamic); 
				components::checkbox("Thirdperson", &cvars::visuals.effects_thirdperson, &cvars::visuals.effects_thirdperson_key); 
				components::keybind("Thirdperson key", &cvars::visuals.effects_thirdperson_key); 
				components::checkbox("Bullet tracers", &cvars::visuals.local_bullet_tracers); 
				components::checkbox("Hotkey list", &cvars::visuals.local_hotkey_list); 
			});
		}
		else
		{
			panel("World & Removals", a, ImVec2(ImMin(content_w, 420.f), 340.f), [&]() 
			{ 
				components::checkbox("Remove smoke", &cvars::visuals.remove_smoke); 
				components::checkbox("Remove scope", &cvars::visuals.remove_scope); 
				components::checkbox("Remove recoil", &cvars::visuals.remove_visual_recoil); 
				components::checkbox("Watermark", &cvars::visuals.watermark); 
				components::checkbox("Notifications", &cvars::visuals.main_notifications); 
				components::checkbox("Stream mode", &cvars::visuals.streamer_mode); 
			});
		}
	}
	else if (m_iSelectedTab == MOVEMENT)
	{
		if (section[MOVEMENT] == 0)
		{
			panel("Main Movement", a, ImVec2(ImMin(content_w, 420.f), 380.f), [&]() 
			{ 
				components::checkbox("Enabled", &cvars::kreedz.active, &cvars::kreedz.key); 
				components::keybind("Key", &cvars::kreedz.key); 
				components::checkbox("Bunnyhop", &cvars::kreedz.bunnyhop); 
				components::checkbox("Ground strafe", &cvars::kreedz.groundstrafe); 
				components::slider_float("Ground gain", &cvars::kreedz.groundstrafe_gain, 0.f, 1.f, "%.2f"); 
				components::checkbox("Fast run", &cvars::kreedz.fastrun, &cvars::kreedz.fastrun_key); 
				components::keybind("Fast run key", &cvars::kreedz.fastrun_key); 
			});
		}
		else if (section[MOVEMENT] == 1)
		{
			panel("Strafes & Helpers", a, ImVec2(ImMin(content_w, 420.f), 370.f), [&]() 
			{ 
				components::checkbox("Rage strafe", &cvars::kreedz.rage_strafe); 
				components::checkbox("Legit strafe", &cvars::kreedz.legit_strafe); 
				components::slider_float("Legit speed", &cvars::kreedz.legit_strafe_speed, 1.f, 100.f, "%.1f", "%"); 
				components::checkbox("DeGen", &cvars::kreedz.degen, &cvars::kreedz.degen_key); 
				components::keybind("DeGen key", &cvars::kreedz.degen_key); 
				components::slider_float("DeGen power", &cvars::kreedz.degen_power, 50.f, 1000.f, "%.0f"); 
			});
		}
		else
		{
			panel("Movement Exploits", a, ImVec2(ImMin(content_w, 420.f), 310.f), [&]() 
			{ 
				components::checkbox("Jump bug", &cvars::kreedz.jumpbug); 
				components::checkbox("Edge bug", &cvars::kreedz.edgebug); 
				components::checkbox("Wall bug", &cvars::kreedz.wallbug); 
				components::checkbox("Auto jump of fall", &cvars::kreedz.auto_jof); 
				components::slider_float("JOF distance", &cvars::kreedz.auto_jof_min_distance, 1.f, 16.f, "%.1f", "units"); 
			});
		}
	}
	else if (m_iSelectedTab == MISC)
	{
		if (section[MISC] == 0)
		{
			panel("Exploits", a, ImVec2(ImMin(content_w, 420.f), 400.f), [&]() 
			{ 
				components::checkbox("Doubletap", &cvars::misc.doubletap, &cvars::misc.doubletap_key); 
				components::keybind("Doubletap key", &cvars::misc.doubletap_key); 
				components::slider_int("Doubletap shift", &cvars::misc.doubletap_shift, 1, 4, "%d", "ticks"); 
				components::checkbox("Airstuck", &cvars::misc.airstuck, &cvars::misc.airstuck_key); 
				components::keybind("Airstuck key", &cvars::misc.airstuck_key); 
				components::checkbox("Fakelatency", &cvars::misc.fakelatency); 
				components::slider_int("Latency", &cvars::misc.fakelatency_amount, 0, 500, "%d", "ms"); 
			});
		}
		else if (section[MISC] == 1)
		{
			panel("Automation", a, ImVec2(ImMin(content_w, 420.f), 350.f), [&]() 
			{ 
				components::checkbox("Automatic reload", &cvars::misc.automatic_reload); 
				components::checkbox("Automatic pistol", &cvars::misc.automatic_pistol); 
				components::checkbox("No spread", &cvars::misc.nospread); 
				components::checkbox("Unlock FPS", &cvars::misc.fps_unlock); 
				components::checkbox("Frame skip", &cvars::misc.frame_skip); 
				components::slider_int("Skip amount", &cvars::misc.frame_skip_amount, 1, 10, "%d", "frames"); 
			});
		}
		else if (section[MISC] == 2)
		{
			panel("Knifebot", a, ImVec2(ImMin(content_w, 420.f), 380.f), [&]() 
			{ 
				components::checkbox("Enabled", &cvars::misc.kb_enabled, &cvars::misc.kb_key); 
				components::keybind("Key", &cvars::misc.kb_key); 
				components::checkbox("Friendly fire", &cvars::misc.kb_friendly_fire); 
				components::slider_float("Maximum FOV", &cvars::misc.kb_fov, 0.f, 180.f, "%.1f", "°"); 
				components::slider_float("Swing distance", &cvars::misc.kb_swing_distance, 16.f, 64.f, "%.1f", "units"); 
				components::slider_float("Stab distance", &cvars::misc.kb_stab_distance, 16.f, 64.f, "%.1f", "units"); 
			});
		}
		else if (section[MISC] == 3)
		{
			panel("Other Features", a, ImVec2(ImMin(content_w, 420.f), 350.f), [&]() 
			{ 
				components::checkbox("Auto block", &cvars::misc.auto_block, &cvars::misc.auto_block_key); 
				components::keybind("Auto block key", &cvars::misc.auto_block_key); 
				components::checkbox("Auto boost", &cvars::misc.auto_boost); 
				components::checkbox("Block MOTD", &cvars::misc.motd_block); 
				components::checkbox("Sandbox mode", &cvars::misc.sandbox_enabled); 
				components::checkbox("SteamID spoofer", &cvars::misc.steamid_spoofer); 
			});
		}
		else
		{
			panel("Theme & Colors", a, ImVec2(ImMin(content_w, 420.f), 280.f), [&]()
			{
				static float col[4] = { accent.x, accent.y, accent.z, accent.w };
				if (components::color_edit("Accent Color", col))
				{
					components::set_accent_color(ImVec4(col[0], col[1], col[2], col[3]));
				}
				ImGui::Spacing();
				if (components::button("Pink / Rose (Default)", ImVec2(-1, 24)))
				{
					components::set_accent_color(ImVec4(0.76f, 0.60f, 0.64f, 1.f));
					col[0] = 0.76f; col[1] = 0.60f; col[2] = 0.64f; col[3] = 1.f;
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
		}
	}
	else if (m_iSelectedTab == CONFIGS)
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
		g_Console.Draw("##console", nullptr);
	}

	ImGui::EndChild();
	if (g_pPrimTextFont)
		ImGui::PopFont();
	ImGui::PopStyleColor(8);
	ImGui::PopStyleVar(3);
	m_pGui->End();
}

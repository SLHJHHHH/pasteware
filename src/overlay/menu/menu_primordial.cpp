#include "framework.h"

namespace
{
	void panel(const char* name, const ImVec2& pos, const ImVec2& size, const std::function<void()>& draw)
	{
		ImGui::SetCursorPos(pos);
		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.105f, 0.105f, 0.112f, 1.f));
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.18f, 0.18f, 0.19f, 1.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.f, 10.f));
		ImGui::BeginChild(name, size, true);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.91f, 0.91f, 0.92f, 1.f));
		ImGui::TextUnformatted(name);
		ImGui::PopStyleColor();
		ImGui::Separator();
		draw();
		ImGui::EndChild();
		ImGui::PopStyleVar();
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
			GImGui->CurrentWindow->DrawList->AddRectFilled(min, min + ImVec2(2.f, 32.f), ImColor(193, 154, 164));
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

	bool checkbox_int(const char* label, int* value)
	{
		bool enabled = *value != 0;
		if (!ImGui::Checkbox(label, &enabled))
			return false;
		*value = enabled ? 1 : 0;
		return true;
	}
}

void CMenu::Draw()
{
	enum { AIMBOT, ANTIAIM, VISUALS, MOVEMENT, MISC, CONFIGS, CONSOLE };
	static int section[7]{};
	static int weapon_group{};
	static int weapon{};
	static int current_weapon = WEAPON_NONE;
	static bool current_weapon_auto{};
	static std::vector<TabWidgetsData> main = { { "Aimbot", 0.f }, { "Antiaim", 0.f }, { "Visuals", 0.f }, { "Movement", 0.f }, { "Misc", 0.f }, { "Config", 0.f }, { "Console", 0.f } };

	if (!m_bIsOpened)
		return;

	if (!m_pGui->Begin("##pasteware", ImVec2(876.f, 623.f)))
		return;

	m_WindowPos = GImGui->CurrentWindow->Pos;
	m_WindowSize = GImGui->CurrentWindow->Size;
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 3.f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(7.f, 4.f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.82f, 0.82f, 0.84f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0.43f, 0.43f, 0.45f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.16f, 0.16f, 0.17f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.21f, 0.21f, 0.22f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.24f, 0.24f, 0.25f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.76f, 0.60f, 0.64f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.76f, 0.60f, 0.64f, 1.f));
	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.90f, 0.73f, 0.77f, 1.f));
	if (g_pPrimTextFont)
		ImGui::PushFont(g_pPrimTextFont);
	m_pGui->TabBackground();
	m_pGui->TabList(main, m_iSelectedTab, m_iHoveredTab);

	ImGui::SetCursorPos(ImVec2(5.f, 70.f));
	ImGui::BeginChild("##prim_sidebar", ImVec2(136.f, 465.f), false);
	switch (m_iSelectedTab)
	{
	case AIMBOT: tabs({ "Legit", "Rage", "Trigger", "Weapon" }, section[AIMBOT]); break;
	case ANTIAIM: tabs({ "Global", "Stand", "Move", "Fakelag" }, section[ANTIAIM]); break;
	case VISUALS: tabs({ "ESP", "Chams", "Local", "World" }, section[VISUALS]); break;
	case MOVEMENT: tabs({ "Main", "Strafes", "Bugs" }, section[MOVEMENT]); break;
	case MISC: tabs({ "Exploits", "Automation", "Knifebot", "Other" }, section[MISC]); break;
	case CONFIGS: tabs({ "Configurations", "Binds" }, section[CONFIGS]); break;
	case CONSOLE: tabs({ "Console" }, section[CONSOLE]); break;
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(153.f, 70.f));
	ImGui::BeginChild("##prim_content", ImVec2(698.f, 465.f), false);
	const ImVec2 a(0.f, 0.f), b(332.f, 0.f), c(0.f, 0.f);

	if (m_iSelectedTab == AIMBOT)
	{
		if (section[AIMBOT] == 0)
		{
			panel("Primary", a, ImVec2(320.f, 290.f), [&]()
			{
				ImGui::Checkbox("Enabled", &cvars::legitbot.active);
				m_pGui->Key(&cvars::legitbot.aim_key, "Aim key");
				ImGui::Checkbox("Friendly fire", &cvars::legitbot.friendly_fire);
				ImGui::Checkbox("Automatic scope", &cvars::legitbot.aim_auto_scope);
				ImGui::Checkbox("Smoke check", &cvars::legitbot.aim_smoke_check);
				ImGui::Checkbox("Dynamic FOV", &cvars::legitbot.aim_dynamic_fov);
			});
			panel("Target", b, ImVec2(320.f, 360.f), [&]()
			{
				ImGui::SliderInt("Head weight", &cvars::legitbot.target_weight_head, 1, 100);
				ImGui::SliderInt("Chest weight", &cvars::legitbot.target_weight_chest, 1, 100);
				ImGui::SliderInt("Stomach weight", &cvars::legitbot.target_weight_stomach, 1, 100);
				ImGui::SliderInt("Arms weight", &cvars::legitbot.target_weight_arms, 1, 100);
				ImGui::SliderInt("Legs weight", &cvars::legitbot.target_weight_legs, 1, 100);
			});
		}
		else if (section[AIMBOT] == 1)
		{
			panel("Rage", a, ImVec2(320.f, 260.f), [&]()
			{
				ImGui::Checkbox("Enabled", &cvars::ragebot.active);
				m_pGui->Key(&cvars::ragebot.raim_key, "Aim key");
				ImGui::Checkbox("Friendly fire", &cvars::ragebot.friendly_fire);
				ImGui::Checkbox("Automatic fire", &cvars::ragebot.raim_auto_fire);
				ImGui::SliderFloat("Maximum FOV", &cvars::ragebot.raim_fov, 0.f, 180.f);
			});
			panel("Weapon", b, ImVec2(320.f, 260.f), [&]()
			{
				ImGui::Checkbox("Enabled", &cvars::weapons[WEAPON_NONE].raim_enabled);
				ImGui::Checkbox("Automatic penetration", &cvars::weapons[WEAPON_NONE].raim_auto_penetration);
				ImGui::SliderInt("Minimum damage", &cvars::weapons[WEAPON_NONE].raim_auto_penetration_min_damage, 1, 100);
			});
		}
		else if (section[AIMBOT] == 2)
		{
			panel("Trigger", a, ImVec2(420.f, 260.f), [&]()
			{
				m_pGui->Key(&cvars::legitbot.trigger_key, "Trigger key");
				ImGui::Checkbox("Accurate traces", &cvars::legitbot.trigger_accurate_traces);
				ImGui::Checkbox("Only scoped", &cvars::legitbot.trigger_only_scoped);
				ImGui::Checkbox("Dynamic", &cvars::legitbot.trigger_dynamic);
				ImGui::SliderInt("Shot delay", &cvars::legitbot.trigger_shot_delay, 0, 1000);
			});
		}
		else
		{
			panel("Weapon profile", a, ImVec2(652.f, 250.f), [&]()
			{
				ImGui::Checkbox("Current weapon", &current_weapon_auto);
				const char* groups[] = { "Global", "Pistol", "SMG", "Rifle", "Shotgun", "Sniper" };
				ImGui::Combo("Group", &weapon_group, groups, IM_ARRAYSIZE(groups));
				current_weapon = WEAPON_NONE;
				ImGui::Checkbox("Aim enabled", &cvars::weapons[current_weapon].aim_enabled);
				ImGui::SliderFloat("Aim FOV", &cvars::weapons[current_weapon].aim_fov, 0.f, 180.f);
			});
		}
	}
	else if (m_iSelectedTab == ANTIAIM)
	{
		if (section[ANTIAIM] == 0)
			panel("Global", a, ImVec2(420.f, 300.f), [&]() { const char* roll[] = { "None", "Sideways 50", "Sideways 90", "Sideways 180", "Static" }; ImGui::Checkbox("Enabled", &cvars::ragebot.aa_enabled); ImGui::Checkbox("Teammates", &cvars::ragebot.aa_teammates); ImGui::Combo("Roll", &cvars::ragebot.aa_roll, roll, IM_ARRAYSIZE(roll)); ImGui::SliderFloat("Static roll", &cvars::ragebot.aa_roll_static, -180.f, 180.f); ImGui::Checkbox("Untrusted checks", &cvars::ragebot.aa_untrusted_checks); });
		else if (section[ANTIAIM] == 1)
			panel("Stand", a, ImVec2(420.f, 300.f), [&]() { const char* pitch[] = { "None", "Down", "Up", "Fake down", "Fake up", "Jitter", "Fake jitter" }; const char* yaw[] = { "None", "Backwards", "Sideways 90", "Sideways 140", "Static", "Local view", "Desync" }; ImGui::Combo("Pitch", &cvars::ragebot.aa_stand_pitch, pitch, IM_ARRAYSIZE(pitch)); ImGui::Combo("Yaw", &cvars::ragebot.aa_stand_yaw, yaw, IM_ARRAYSIZE(yaw)); ImGui::SliderFloat("Static yaw", &cvars::ragebot.aa_stand_yaw_static, -180.f, 180.f); ImGui::Checkbox("Desync helper", &cvars::ragebot.aa_stand_desync_helper); });
		else if (section[ANTIAIM] == 2)
			panel("Move", a, ImVec2(420.f, 210.f), [&]() { const char* pitch[] = { "None", "Down", "Up", "Fake down", "Fake up", "Jitter", "Fake jitter" }; const char* yaw[] = { "None", "Backwards", "Local view", "Gait sideways" }; ImGui::Combo("Pitch", &cvars::ragebot.aa_move_pitch, pitch, IM_ARRAYSIZE(pitch)); ImGui::Combo("Yaw", &cvars::ragebot.aa_move_yaw, yaw, IM_ARRAYSIZE(yaw)); });
		else
			panel("Fakelag", a, ImVec2(420.f, 280.f), [&]() { const char* type[] = { "Maximum", "Break lag compensation" }; ImGui::Checkbox("Enabled", &cvars::ragebot.fakelag_enabled); ImGui::Combo("Type", &cvars::ragebot.fakelag_type, type, IM_ARRAYSIZE(type)); ImGui::SliderInt("Choke limit", &cvars::ragebot.fakelag_choke_limit, 1, MAX_TOTAL_CMDS); ImGui::Checkbox("While shooting", &cvars::ragebot.fakelag_while_shooting); ImGui::Checkbox("On peek", &cvars::ragebot.fakelag_on_peek); });
	}
	else if (m_iSelectedTab == VISUALS)
	{
		if (section[VISUALS] == 0)
			panel("ESP", a, ImVec2(420.f, 410.f), [&]() { const char* pos[] = { "Off", "Left", "Right", "Top", "Bottom" }; ImGui::Checkbox("Enabled", &cvars::visuals.active); ImGui::Checkbox("Player ESP", &cvars::visuals.esp_player); ImGui::Combo("Box", &cvars::visuals.esp_player_box, pos, IM_ARRAYSIZE(pos)); ImGui::Checkbox("Box outline", &cvars::visuals.esp_player_box_outline); ImGui::Combo("Health", &cvars::visuals.esp_player_health, pos, IM_ARRAYSIZE(pos)); ImGui::Combo("Name", &cvars::visuals.esp_player_name, pos, IM_ARRAYSIZE(pos)); ImGui::Combo("Weapon text", &cvars::visuals.esp_player_weapon_text, pos, IM_ARRAYSIZE(pos)); ImGui::Checkbox("Skeleton", &cvars::visuals.esp_player_skeleton); });
		else if (section[VISUALS] == 1)
			panel("Chams", a, ImVec2(420.f, 280.f), [&]() { checkbox_int("Players", &cvars::visuals.colored_models_players); ImGui::Checkbox("Behind wall", &cvars::visuals.colored_models_players_behind_wall); ImGui::Checkbox("Wireframe", &cvars::visuals.colored_models_players_wireframe); checkbox_int("Hands", &cvars::visuals.colored_models_hands); checkbox_int("Backtrack", &cvars::visuals.colored_models_backtrack); });
		else if (section[VISUALS] == 2)
			panel("Local", a, ImVec2(420.f, 300.f), [&]() { ImGui::Checkbox("Crosshair", &cvars::visuals.crosshair_enabled); ImGui::Checkbox("Dynamic crosshair", &cvars::visuals.crosshair_dynamic); checkbox_int("Thirdperson", &cvars::visuals.effects_thirdperson); m_pGui->Key(&cvars::visuals.effects_thirdperson_key, "Thirdperson key"); ImGui::Checkbox("Bullet tracers", &cvars::visuals.local_bullet_tracers); ImGui::Checkbox("Hotkey list", &cvars::visuals.local_hotkey_list); });
		else
			panel("World", a, ImVec2(420.f, 320.f), [&]() { ImGui::Checkbox("Remove smoke", &cvars::visuals.remove_smoke); ImGui::Checkbox("Remove scope", &cvars::visuals.remove_scope); ImGui::Checkbox("Remove recoil", &cvars::visuals.remove_visual_recoil); ImGui::Checkbox("Watermark", &cvars::visuals.watermark); ImGui::Checkbox("Notifications", &cvars::visuals.main_notifications); ImGui::Checkbox("Stream mode", &cvars::visuals.streamer_mode); });
	}
	else if (m_iSelectedTab == MOVEMENT)
	{
		if (section[MOVEMENT] == 0)
			panel("Main", a, ImVec2(420.f, 340.f), [&]() { ImGui::Checkbox("Enabled", &cvars::kreedz.active); m_pGui->Key(&cvars::kreedz.key, "Key"); ImGui::Checkbox("Bunnyhop", &cvars::kreedz.bunnyhop); ImGui::Checkbox("Ground strafe", &cvars::kreedz.groundstrafe); ImGui::SliderFloat("Ground gain", &cvars::kreedz.groundstrafe_gain, 0.f, 1.f); ImGui::Checkbox("Fast run", &cvars::kreedz.fastrun); m_pGui->Key(&cvars::kreedz.fastrun_key, "Fast run key"); });
		else if (section[MOVEMENT] == 1)
			panel("Strafes", a, ImVec2(420.f, 330.f), [&]() { ImGui::Checkbox("Rage strafe", &cvars::kreedz.rage_strafe); ImGui::Checkbox("Legit strafe", &cvars::kreedz.legit_strafe); ImGui::SliderFloat("Legit speed", &cvars::kreedz.legit_strafe_speed, 1.f, 100.f); ImGui::Checkbox("DeGen", &cvars::kreedz.degen); m_pGui->Key(&cvars::kreedz.degen_key, "DeGen key"); ImGui::SliderFloat("DeGen power", &cvars::kreedz.degen_power, 50.f, 1000.f); });
		else
			panel("Bugs", a, ImVec2(420.f, 280.f), [&]() { ImGui::Checkbox("Jump bug", &cvars::kreedz.jumpbug); ImGui::Checkbox("Edge bug", &cvars::kreedz.edgebug); ImGui::Checkbox("Wall bug", &cvars::kreedz.wallbug); ImGui::Checkbox("Auto jump of fall", &cvars::kreedz.auto_jof); ImGui::SliderFloat("JOF distance", &cvars::kreedz.auto_jof_min_distance, 1.f, 16.f); });
	}
	else if (m_iSelectedTab == MISC)
	{
		if (section[MISC] == 0)
			panel("Exploits", a, ImVec2(420.f, 360.f), [&]() { ImGui::Checkbox("Doubletap", &cvars::misc.doubletap); m_pGui->Key(&cvars::misc.doubletap_key, "Doubletap key"); ImGui::SliderInt("Doubletap shift", &cvars::misc.doubletap_shift, 1, 4); ImGui::Checkbox("Airstuck", &cvars::misc.airstuck); m_pGui->Key(&cvars::misc.airstuck_key, "Airstuck key"); ImGui::Checkbox("Fakelatency", &cvars::misc.fakelatency); ImGui::SliderInt("Latency", &cvars::misc.fakelatency_amount, 0, 500); });
		else if (section[MISC] == 1)
			panel("Automation", a, ImVec2(420.f, 320.f), [&]() { ImGui::Checkbox("Automatic reload", &cvars::misc.automatic_reload); ImGui::Checkbox("Automatic pistol", &cvars::misc.automatic_pistol); ImGui::Checkbox("No spread", &cvars::misc.nospread); ImGui::Checkbox("Unlock FPS", &cvars::misc.fps_unlock); ImGui::Checkbox("Frame skip", &cvars::misc.frame_skip); ImGui::SliderInt("Skip amount", &cvars::misc.frame_skip_amount, 1, 10); });
		else if (section[MISC] == 2)
			panel("Knifebot", a, ImVec2(420.f, 350.f), [&]() { ImGui::Checkbox("Enabled", &cvars::misc.kb_enabled); m_pGui->Key(&cvars::misc.kb_key, "Key"); ImGui::Checkbox("Friendly fire", &cvars::misc.kb_friendly_fire); ImGui::SliderFloat("Maximum FOV", &cvars::misc.kb_fov, 0.f, 180.f); ImGui::SliderFloat("Swing distance", &cvars::misc.kb_swing_distance, 16.f, 64.f); ImGui::SliderFloat("Stab distance", &cvars::misc.kb_stab_distance, 16.f, 64.f); });
		else
			panel("Other", a, ImVec2(420.f, 320.f), [&]() { ImGui::Checkbox("Auto block", &cvars::misc.auto_block); m_pGui->Key(&cvars::misc.auto_block_key, "Auto block key"); ImGui::Checkbox("Auto boost", &cvars::misc.auto_boost); ImGui::Checkbox("Block MOTD", &cvars::misc.motd_block); ImGui::Checkbox("Sandbox", &cvars::misc.sandbox_enabled); ImGui::Checkbox("SteamID spoofer", &cvars::misc.steamid_spoofer); });
	}
	else if (m_iSelectedTab == CONFIGS)
	{
		panel("Configurations", a, ImVec2(420.f, 410.f), [&]() { m_pSettingsList->RefreshSettingsList(); m_pGui->ListBox("##settings", &m_pSettingsList->m_selected_settings, m_pSettingsList->m_settings_list, 10); if (ImGui::Button("Load", ImVec2(196.f, 24.f))) g_pSettings->Load(m_pSettingsList->GetFocusSettings()); if (ImGui::Button("Save", ImVec2(196.f, 24.f))) g_pSettings->Save(m_pSettingsList->GetFocusSettings()); });
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

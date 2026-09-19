#include "framework.h"

constexpr auto TOGGLE_STATUS_TIME = 4.0;

ToggleStatusData g_ToggleStatus;
std::deque<BulletTracerData> g_BulletTracers;
HitMarkerData g_HitMarker;
std::deque<DamageLogData> g_DamageLog;

void RegisterHitEvent(int index, int damage, bool headshot)
{
	if (!Game::IsConnected())
		return;

	if (cvars::visuals.hitmarker)
	{
		g_HitMarker.headshot = headshot;
		g_HitMarker.timestamp = static_cast<float>(client_state->time);
	}

	if (cvars::visuals.damage_log)
	{
		DamageLogData entry;

		entry.name = (index > 0 && index <= MAX_CLIENTS) ? g_Player[index]->m_szPrintName : "";
		entry.damage = damage;
		entry.health = (index > 0 && index <= MAX_CLIENTS) ? g_Player[index]->m_iHealth : 0;
		entry.headshot = headshot;
		entry.timestamp = client_state->time;

		g_DamageLog.push_back(entry);

		while (g_DamageLog.size() > 8)
			g_DamageLog.pop_front();
	}

	if (cvars::visuals.hitsound)
	{
		static const char* sounds[] =
		{
			"buttons/blip1.wav",
			"buttons/blip2.wav",
			"weapons/bullet_hit1.wav",
			"player/pl_pain2.wav"
		};

		const int type = std::clamp(cvars::visuals.hitsound_type, 0, static_cast<int>(IM_ARRAYSIZE(sounds)) - 1);

		char sound[MAX_PATH] = { '\0' };
		strcpy_s(sound, sounds[type]);

		const float volume = std::clamp(cvars::visuals.hitsound_volume, 0.f, 1.f);

		if (headshot)
			g_Engine.pfnPlaySoundByNameAtPitch(sound, volume, std::clamp(cvars::visuals.hitsound_headshot_pitch, 50, 200));
		else
			g_Engine.pfnPlaySoundByName(sound, volume);
	}
}

CDrawLocal::CDrawLocal()
	: m_pDefaultCrosshair(std::make_unique<CDefaultCrosshair>())
{
}

static ImVec2 GetPunchPosition()
{
	ImVec2 punchangle_position;

	punchangle_position = GImGui->IO.DisplaySize * 0.5f;

	Vector vecForward, vecPunchangle;

	g_Local->m_QAngles.AngleVectors(&vecForward, NULL, NULL);

	vecPunchangle = g_Local->m_vecEyePos + vecForward * g_Weapon->m_flDistance;

	Vector2D vecScreenPos;

	if (Game::WorldToScreen(vecPunchangle, vecScreenPos))
	{
		punchangle_position.x = GImGui->IO.DisplaySize.x - vecScreenPos.x;
		punchangle_position.y = GImGui->IO.DisplaySize.y - vecScreenPos.y;
	}

	return punchangle_position;
}

void CDrawLocal::Overlay()
{
	if (g_pIGameUI->IsGameUIActive())
		return;

	if (!g_Local->m_bIsDead)
	{
		if (cvars::visuals.esp_other_local_spread_circle)
			SpreadCircle();

		if (cvars::visuals.esp_other_local_aim_fov)
			AimFOV();

		if (cvars::visuals.esp_other_local_recoil_point)
			RecoilPoint();

		if (cvars::visuals.esp_other_local_spread_point)
			SpreadPoint();

		if (cvars::visuals.esp_other_local_toggle_status)
			ToggleStatus();
	}

	if (cvars::visuals.local_bullet_tracers)
		BulletTracers();

	if (cvars::visuals.crosshair_enabled) CustomCrosshair(); if (cvars::visuals.watermark || cvars::visuals.watermark_credits) Watermark(); if (cvars::visuals.local_function_status) FunctionStatus(); if (cvars::visuals.local_hotkey_list) HotkeyList(); if (cvars::visuals.local_statistics) Statistics(); if (cvars::visuals.esp_other_local_sniper_crosshair)
		SniperCrosshair();

	if (cvars::visuals.hitmarker)
		HitMarker();

	if (cvars::visuals.damage_log)
		DamageLog();

	if (cvars::visuals.indicators)
		Indicators();

	if (cvars::visuals.spectator_list)
		SpectatorList();
}

void CDrawLocal::SniperCrosshair()
{
	if (g_Weapon.IsSniper() && !g_Local->m_bIsScoped)
		m_pDefaultCrosshair->Draw();
}

void CDrawLocal::AimFOV()
{
	float flCurrentFOV = g_pLegitBot.get() ? g_pLegitBot->m_flCurrentFOV : g_pRageBot.get() ? cvars::ragebot.raim_fov : 0.f;

	if (flCurrentFOV && flCurrentFOV < g_Local->m_iRealFOV)
	{
		const auto punchangle_position = GetPunchPosition();
		const auto radius = tan(DEG2RAD(flCurrentFOV * 0.5f)) / tan(DEG2RAD(g_Local->m_iRealFOV * 0.5f)) * GImGui->IO.DisplaySize.x;

		for (float angle = 0.f, step = 1.5f; angle < 360.f; angle += step)
		{
			const float current_angle = angle * IM_PI / 180.f;
			const float next_angle = (angle + step) * IM_PI / 180.f;

			ImVec2 a = punchangle_position;
			ImVec2 b = punchangle_position + ImVec2(cos(current_angle), sin(current_angle)) * radius;
			ImVec2 c = punchangle_position + ImVec2(cos(next_angle), sin(next_angle)) * radius;

			ImVec2 uv_white = ImGui::GetFontTexUvWhitePixel();

			GImGui->CurrentWindow->DrawList->PrimReserve(3, 3);
			GImGui->CurrentWindow->DrawList->PrimVtx(a, uv_white, ImColor(cvars::visuals.esp_other_local_aim_fov_color2));
			GImGui->CurrentWindow->DrawList->PrimVtx(b, uv_white, ImColor(cvars::visuals.esp_other_local_aim_fov_color));
			GImGui->CurrentWindow->DrawList->PrimVtx(c, uv_white, ImColor(cvars::visuals.esp_other_local_aim_fov_color));
		}

		ImColor color = cvars::visuals.esp_other_local_aim_fov_color;

		g_pRenderer->AddCircle(punchangle_position, radius, color, 100, 2);
	}
}

void CDrawLocal::RecoilPoint()
{
	if (g_Local->m_bIsDead)
		return;

	if (!g_Weapon.IsGun())
		return;

	if (client_state->punchangle.IsZero2D())
		return;

	const auto punchangle_position = GetPunchPosition();

	g_pRenderer->AddRectFilled(punchangle_position - 2, punchangle_position + 2, cvars::visuals.esp_other_local_recoil_point_color);
}

void CDrawLocal::SpreadPoint()
{
	if (g_Local->m_bIsDead)
		return;

	if (!g_Weapon.IsGun())
		return;

	if (!g_Weapon->m_flSpread)
		return;

	QAngle QSpreadAngles = g_Local->m_QAngles;

	g_pNoSpread->GetSpreadOffset(g_Weapon->m_iRandomSeed, 1, QSpreadAngles, QSpreadAngles, NOSPREAD_PITCH_YAW);

	Vector vecForward, vecSpread;

	QSpreadAngles.AngleVectors(&vecForward, NULL, NULL);

	vecSpread = g_Local->m_vecEyePos + vecForward * g_Weapon->m_flDistance;

	Vector2D vecScreenPos;

	if (Game::WorldToScreen(vecSpread, vecScreenPos))
	{
		ImVec2 spread_position = GImGui->IO.DisplaySize - ImVec2(vecScreenPos.x, vecScreenPos.y);

		g_pRenderer->AddRectFilled(spread_position - 2, spread_position + 2, cvars::visuals.esp_other_local_spread_point_color);
	}
}

void CDrawLocal::SpreadCircle()
{
	enum { previous, current, calculated, max_count };

	static double radius[max_count], change_timestamp;

	const auto current_radius = g_Weapon->m_flSpread > 0.015 ? 1000.0 * g_Weapon->m_flSpread / g_Local->m_iRealFOV * DEFAULT_FOV : 0.0;
	constexpr auto animation_time = 0.2;

	if (radius[current] != current_radius)
	{
		if (change_timestamp + animation_time >= client_state->time)
			radius[previous] = radius[calculated];

		radius[current] = current_radius;
		change_timestamp = client_state->time;
	}

	if (change_timestamp + animation_time >= client_state->time)
	{
		radius[calculated] = static_cast<float>(Math::Interp(change_timestamp, client_state->time,
			change_timestamp + animation_time, radius[previous], radius[current]));
	}
	else
	{
		radius[calculated] = radius[current];
		radius[previous] = radius[current];
	}

	if (radius[calculated] < 5.f)
		return;

	const auto punchangle_position = GetPunchPosition();

	for (float angle = 0.f, step = 1.5f; angle < 360.f; angle += step)
	{
		const float current_angle = angle * IM_PI / 180.f;
		const float next_angle = (angle + step) * IM_PI / 180.f;

		ImVec2 a = punchangle_position;
		ImVec2 b = punchangle_position + ImVec2(cos(current_angle), sin(current_angle)) * static_cast<float>(radius[calculated]);
		ImVec2 c = punchangle_position + ImVec2(cos(next_angle), sin(next_angle)) * static_cast<float>(radius[calculated]);

		ImVec2 uv_white = ImGui::GetFontTexUvWhitePixel();

		GImGui->CurrentWindow->DrawList->PrimReserve(3, 3);
		GImGui->CurrentWindow->DrawList->PrimVtx(a, uv_white, ImColor(cvars::visuals.esp_other_local_spread_circle_color2));
		GImGui->CurrentWindow->DrawList->PrimVtx(b, uv_white, ImColor(cvars::visuals.esp_other_local_spread_circle_color));
		GImGui->CurrentWindow->DrawList->PrimVtx(c, uv_white, ImColor(cvars::visuals.esp_other_local_spread_circle_color));
	}

	ImColor color = cvars::visuals.esp_other_local_spread_circle_color;

	g_pRenderer->AddCircle(punchangle_position, static_cast<float>(radius[calculated]), color, 100, 2);
}

void CDrawLocal::ToggleStatus()
{
	constexpr auto FADEOUT_TIME = 1.0;

	const auto text_size = GImGui->Font->CalcTextSizeA(GImGui->Font->FontSize, FLT_MAX, 0, g_ToggleStatus.name.c_str());

	ImVec2 pos = ImVec2(GImGui->IO.DisplaySize.x * 0.5f, GImGui->IO.DisplaySize.y * 0.25f);

	ImColor color = g_ToggleStatus.status ? cvars::visuals.esp_other_local_toggle_status_color : cvars::visuals.esp_other_local_toggle_status_color2;

	if (g_ToggleStatus.timestamp + (TOGGLE_STATUS_TIME - FADEOUT_TIME) < client_state->time)
	{
		color.value.w = static_cast<float>(Math::Interp(g_ToggleStatus.timestamp + (TOGGLE_STATUS_TIME - FADEOUT_TIME),
			client_state->time, g_ToggleStatus.timestamp + TOGGLE_STATUS_TIME, 1.00, 0.00));
	}

	ImGui::PushFont(g_pFontList[Verdana_17px]);

	g_pRenderer->AddText(pos, color, FontFlags_CenterX, g_ToggleStatus.status ? "%s: ON" : "%s: OFF", g_ToggleStatus.name.c_str());

	ImGui::PopFont();
}
void CDrawLocal::Watermark()
{

	constexpr const char* credits = "made by warfallen & @DONECKII";

	char buf[256] = { 0 };
	int o = 0;

	if (cvars::visuals.watermark_elements[0]) o += sprintf_s(buf + o, sizeof(buf) - o, "%s | ", "time");
	if (cvars::visuals.watermark_elements[1]) o += sprintf_s(buf + o, sizeof(buf) - o, "%s | ", "pasteware");
	if (cvars::visuals.watermark_elements[2]) o += sprintf_s(buf + o, sizeof(buf) - o, "%i fps | ", (int)(1.f / max(g_Local->m_flFrameTime, 0.001f)));
	if (cvars::visuals.watermark_elements[3]) o += sprintf_s(buf + o, sizeof(buf) - o, "%i ms | ", (int)(client_state->frames[client_state->parsecountmod].latency * 1000.f));

	if (o >= 3)
		buf[o - 3] = 0;

	if (buf[0])
	{
		ImGui::PushFont(g_pFontList[Verdana_17px]);

		const ImVec2 text_size = ImGui::CalcTextSize(buf);
		const ImVec2 pos = ImVec2(GImGui->IO.DisplaySize.x - text_size.x - 12.f, 8.f);

		g_pRenderer->AddText(pos, ImColor(255, 255, 255), FontFlags_None, "%s", buf);

		ImGui::PopFont();
	}

	if (cvars::visuals.watermark_credits)
	{
		ImGui::PushFont(g_pFontList[Verdana_13px]);

		const ImVec2 text_size = ImGui::CalcTextSize(credits);
		const ImVec2 pos = ImVec2(GImGui->IO.DisplaySize.x - text_size.x - 12.f, GImGui->IO.DisplaySize.y - text_size.y - 8.f);

		g_pRenderer->AddText(pos, ImColor(255, 255, 255, 90), FontFlags_None, "%s", credits);

		ImGui::PopFont();
	}
}
void CDrawLocal::CustomCrosshair()
{
if (g_pIGameUI->IsGameUIActive()) return;
ImVec2 c = GImGui->IO.DisplaySize * 0.5f;
int sz = cvars::visuals.crosshair_size, gap = cvars::visuals.crosshair_gap, th = max(1, cvars::visuals.crosshair_thickness);
if (cvars::visuals.crosshair_dynamic) { sz += (int)(g_Weapon->m_flSpread * (float)cvars::visuals.crosshair_dynamic_scale); gap += (int)(g_Local->m_flVelocity * 0.01f); }
ImColor col = ImColor(0,255,0);
g_pRenderer->AddRectFilled(ImVec2(c.x - gap - sz, c.y - th * 0.5f), ImVec2(c.x - gap, c.y + th * 0.5f), col);
g_pRenderer->AddRectFilled(ImVec2(c.x + gap, c.y - th * 0.5f), ImVec2(c.x + gap + sz, c.y + th * 0.5f), col);
g_pRenderer->AddRectFilled(ImVec2(c.x - th * 0.5f, c.y - gap - sz), ImVec2(c.x + th * 0.5f, c.y - gap), col);
g_pRenderer->AddRectFilled(ImVec2(c.x - th * 0.5f, c.y + gap), ImVec2(c.x + th * 0.5f, c.y + gap + sz), col);
if (cvars::visuals.crosshair_dot) g_pRenderer->AddRectFilled(ImVec2(c.x - 1, c.y - 1), ImVec2(c.x + 1, c.y + 1), col);
}
void CDrawLocal::FunctionStatus() { ImGui::PushFont(g_pFontList[Verdana_17px]); g_pRenderer->AddText(ImVec2(12.f, GImGui->IO.DisplaySize.y * 0.4f), ImColor(255,255,0), FontFlags_None, "legit:%s rage:%s", cvars::legitbot.active ? "on" : "off", cvars::ragebot.active ? "on" : "off"); ImGui::PopFont(); }
void CDrawLocal::HotkeyList() { ImGui::PushFont(g_pFontList[Verdana_17px]); int y = (int)(GImGui->IO.DisplaySize.y * 0.45f); if (g_pLegitBot.get() && g_pLegitBot->m_bAimState) { g_pRenderer->AddText(ImVec2(12.f, (float)y), ImColor(255,255,255), FontFlags_None, "aim [on]"); y += 14; } if (g_pRageBot.get() && g_pRageBot->m_bAimState) { g_pRenderer->AddText(ImVec2(12.f, (float)y), ImColor(255,255,255), FontFlags_None, "rage [on]"); y += 14; } if (g_pRageBot.get() && g_pRageBot->m_bForceBody) { g_pRenderer->AddText(ImVec2(12.f, (float)y), ImColor(255,255,255), FontFlags_None, "force body [on]"); y += 14; } if (g_pRageBot.get() && g_pRageBot->m_bForceMinDamage) { g_pRenderer->AddText(ImVec2(12.f, (float)y), ImColor(255,255,255), FontFlags_None, "force dmg [on]"); y += 14; } ImGui::PopFont(); }
void CDrawLocal::Statistics() { char b[128] = { 0 }; int h = g_HitRegister->m_iHits, m = g_HitRegister->m_iMisses, tot = h + m; float pct = tot ? (100.f * h / tot) : 0.f; int o = 0; if (cvars::visuals.local_statistics_elements[0]) o += sprintf_s(b + o, sizeof(b) - o, "hits:%i ", h); if (cvars::visuals.local_statistics_elements[1]) o += sprintf_s(b + o, sizeof(b) - o, "misses:%i ", m); if (cvars::visuals.local_statistics_elements[2]) o += sprintf_s(b + o, sizeof(b) - o, "%.0f%%", pct); ImGui::PushFont(g_pFontList[Verdana_17px]); g_pRenderer->AddText(ImVec2(12.f, GImGui->IO.DisplaySize.y * 0.55f), ImColor(255,255,255), FontFlags_None, "%s", b); ImGui::PopFont(); }

void CDrawLocal::HitMarker()
{
	const float life = max(cvars::visuals.hitmarker_time, 0.05f);
	const float elapsed = static_cast<float>(client_state->time) - g_HitMarker.timestamp;

	if (elapsed < 0.f || elapsed > life)
		return;

	const float alpha = 1.f - (elapsed / life);

	ImColor color = g_HitMarker.headshot ? ImColor(cvars::visuals.hitmarker_headshot_color) : ImColor(cvars::visuals.hitmarker_color);
	color.value.w *= alpha;

	const float size = max(cvars::visuals.hitmarker_size, 2.f) * (1.f + (1.f - alpha) * 0.4f);
	const float inner = size * 0.35f;

	const ImVec2 center = GImGui->IO.DisplaySize * 0.5f;

	g_pRenderer->AddLine(ImVec2(center.x - size, center.y - size), ImVec2(center.x - inner, center.y - inner), color);
	g_pRenderer->AddLine(ImVec2(center.x + size, center.y - size), ImVec2(center.x + inner, center.y - inner), color);
	g_pRenderer->AddLine(ImVec2(center.x - size, center.y + size), ImVec2(center.x - inner, center.y + inner), color);
	g_pRenderer->AddLine(ImVec2(center.x + size, center.y + size), ImVec2(center.x + inner, center.y + inner), color);
}

void CDrawLocal::DamageLog()
{
	const float life = max(cvars::visuals.damage_log_time, 0.5f);

	while (!g_DamageLog.empty() && static_cast<float>(client_state->time) - static_cast<float>(g_DamageLog.front().timestamp) > life)
		g_DamageLog.pop_front();

	if (g_DamageLog.empty())
		return;

	ImGui::PushFont(g_pFontList[Verdana_17px]);

	const float line_height = ImGui::GetFontSize() + 4.f;
	const float padding = 8.f;
	const int pos = std::clamp(cvars::visuals.damage_log_pos, 0, 3);

	float max_width = 0.f;

	for (auto& entry : g_DamageLog)
	{
		const float width = ImGui::CalcTextSize(entry.name.c_str()).x + 110.f;

		if (width > max_width)
			max_width = width;
	}

	const float panel_height = line_height * static_cast<float>(g_DamageLog.size()) + padding;
	const float panel_width = max_width + padding;

	ImVec2 start;

	start.x = (pos == 0 || pos == 2) ? 12.f : GImGui->IO.DisplaySize.x - panel_width - 12.f;
	start.y = (pos == 0 || pos == 1) ? 90.f : GImGui->IO.DisplaySize.y - panel_height - 90.f;

	g_pRenderer->AddRectFilled(start, ImVec2(start.x + panel_width, start.y + panel_height), ImColor(18, 18, 20, 200), 4.f);

	float y = start.y + padding * 0.5f;

	for (auto& entry : g_DamageLog)
	{
		const float alpha = std::clamp(1.f - (static_cast<float>(client_state->time) - static_cast<float>(entry.timestamp)) / life, 0.f, 1.f);

		ImColor color = entry.headshot ? ImColor(cvars::visuals.damage_log_headshot_color) : ImColor(cvars::visuals.damage_log_color);
		color.value.w *= alpha;

		ImColor name_color(210, 210, 215);
		name_color.value.w *= alpha;

		g_pRenderer->AddText(ImVec2(start.x + padding, y), name_color, FontFlags_Shadow, "%s", entry.name.c_str());
		g_pRenderer->AddText(ImVec2(start.x + max_width + padding - 4.f, y), color, FontFlags_Shadow, "-%i (%i hp)%s", entry.damage, entry.health, entry.headshot ? " hs" : "");

		y += line_height;
	}

	ImGui::PopFont();
}

void CDrawLocal::BulletTracers()
{
	while (!g_BulletTracers.empty() && client_state->time - g_BulletTracers.front().time >= cvars::visuals.local_bullet_tracers_time)
		g_BulletTracers.pop_front();

	for (auto& tracer : g_BulletTracers)
	{
		Vector2D ScreenStart, ScreenEnd;

		if (!Game::WorldToScreen(tracer.start, ScreenStart) || !Game::WorldToScreen(tracer.end, ScreenEnd))
			continue;

		const double s1 = tracer.time;
		const double s3 = s1 + cvars::visuals.local_bullet_tracers_time;

		ImColor color = cvars::visuals.local_bullet_tracers_color;
		color.value.w = static_cast<float>(Math::Interp(s1, client_state->time, s3, 1.0, 0.0));

		g_pRenderer->AddLine(ScreenStart, ScreenEnd, color);
	}
}

void CDrawLocal::Indicators()
{
	if (g_pIGameUI->IsGameUIActive() || g_Local->m_bIsDead)
		return;

	struct IndicatorRow
	{
		std::string label;
		std::string value;
		float fill;
	};

	std::deque<IndicatorRow> rows;

	const int choked = g_pMiscellaneous.get() ? g_pMiscellaneous->m_iChokedCommands : 0;
	const int max_commands = max(g_pGlobals->m_nNewCommands, 1);

	if (cvars::visuals.indicators_elements[0])
		rows.push_back({ "choke", std::to_string(choked), std::clamp(static_cast<float>(choked) / static_cast<float>(max_commands), 0.f, 1.f) });

	if (cvars::visuals.indicators_elements[1])
	{
		const int charge = g_pMiscellaneous.get() ? g_pMiscellaneous->m_iDoubleTapCharge : 0;
		const int limit = std::clamp(cvars::misc.doubletap_shift, 1, 4);
		const bool shifting = g_pMiscellaneous.get() && g_pMiscellaneous->m_bDoubleTapShifting;

		std::string value = shifting ? "shift" : (charge ? std::to_string(charge) + "/" + std::to_string(limit) : "empty");

		rows.push_back({ "double tap", value, std::clamp(static_cast<float>(charge) / static_cast<float>(limit), 0.f, 1.f) });
	}

	if (cvars::visuals.indicators_elements[2])
	{
		const bool enabled = cvars::ragebot.fakelag_enabled;
		const int limit = max(cvars::ragebot.fakelag_choke_limit, 1);

		std::string value = enabled ? std::to_string(choked) + "/" + std::to_string(limit) : "off";

		rows.push_back({ "fake lag", value, enabled ? std::clamp(static_cast<float>(choked) / static_cast<float>(limit), 0.f, 1.f) : 0.f });
	}

	if (cvars::visuals.indicators_elements[3])
	{
		const float speed = g_Local->m_flVelocity;

		rows.push_back({ "speed", std::to_string(static_cast<int>(speed)), std::clamp(speed / 300.f, 0.f, 1.f) });
	}

	if (cvars::visuals.indicators_elements[4])
	{
		if (g_pLegitBot && cvars::legitbot.humanizer && cvars::legitbot.humanizer_reaction_max >= cvars::legitbot.humanizer_reaction_min)
		{
			if (g_pLegitBot->m_bHumanizerActive)
			{
				const float reaction = max(g_pLegitBot->m_flHumanizerReaction, 0.001f);

				rows.push_back({ "humanizer", std::to_string(static_cast<int>(reaction * 1000.f)) + "ms", std::clamp(g_pLegitBot->m_flHumanizerTimer / reaction, 0.f, 1.f) });
			}
		}
		else
		{
			const bool active = cvars::ragebot.active && cvars::ragebot.aa_enabled;

			rows.push_back({ "anti-aim", active ? (cvars::ragebot.aa_side ? "right" : "left") : "off", active ? 1.f : 0.f });
		}
	}

	if (rows.empty())
		return;

	ImGui::PushFont(g_pFontList[Verdana_17px]);

	const float line_height = ImGui::GetFontSize() + 6.f;
	const float padding = 10.f;
	const float bar_width = 60.f;
	const float panel_width = 168.f;
	const float panel_height = line_height * static_cast<float>(rows.size()) + padding * 2.f;

	const int pos = std::clamp(cvars::visuals.indicators_pos, 0, 3);

	ImVec2 start;

	if (pos == 0)
		start = ImVec2((GImGui->IO.DisplaySize.x - panel_width) * 0.5f, GImGui->IO.DisplaySize.y - panel_height - 130.f);
	else if (pos == 1)
		start = ImVec2((GImGui->IO.DisplaySize.x - panel_width) * 0.5f, 130.f);
	else if (pos == 2)
		start = ImVec2(12.f, (GImGui->IO.DisplaySize.y - panel_height) * 0.5f);
	else
		start = ImVec2(GImGui->IO.DisplaySize.x - panel_width - 12.f, (GImGui->IO.DisplaySize.y - panel_height) * 0.5f);

	const ImColor accent(cvars::visuals.indicators_color);

	g_pRenderer->AddRectFilled(start, ImVec2(start.x + panel_width, start.y + panel_height), ImColor(18, 18, 20, 205), 5.f);
	g_pRenderer->AddRect(start, ImVec2(start.x + panel_width, start.y + panel_height), accent, false, 5.f, ImDrawCornerFlags_All, 1.f);

	float y = start.y + padding;

	for (auto& row : rows)
	{
		g_pRenderer->AddText(ImVec2(start.x + padding, y), ImColor(205, 205, 212), FontFlags_Shadow, "%s", row.label.c_str());

		const float bar_x = start.x + panel_width - padding - bar_width;
		const float bar_y = y + 4.f;

		g_pRenderer->AddRectFilled(ImVec2(bar_x, bar_y), ImVec2(bar_x + bar_width, bar_y + 7.f), ImColor(38, 38, 42), 3.f);

		if (row.fill > 0.f)
			g_pRenderer->AddRectFilled(ImVec2(bar_x, bar_y), ImVec2(bar_x + bar_width * row.fill, bar_y + 7.f), accent, 3.f);

		ImVec2 value_size = ImGui::CalcTextSize(row.value.c_str());

		g_pRenderer->AddText(ImVec2(bar_x - 6.f - value_size.x, y), accent, FontFlags_Shadow, "%s", row.value.c_str());

		y += line_height;
	}

	ImGui::PopFont();
}

void CDrawLocal::SpectatorList()
{
	if (g_pIGameUI->IsGameUIActive())
		return;

	std::deque<std::string> spectators;

	for (int i = 1; i <= client_state->maxclients; i++)
	{
		if (g_Player[i]->m_bIsLocal)
			continue;

		if (!g_Player[i]->m_bIsConnected)
			continue;

		if (g_Player[i]->m_iObserverState < OBS_CHASE_LOCKED)
			continue;

		if (g_Player[i]->m_iObserverIndex != g_Local->m_iEntIndex)
			continue;

		spectators.push_back(g_Player[i]->m_szPrintName);
	}

	if (spectators.empty())
		return;

	ImGui::PushFont(g_pFontList[Verdana_17px]);

	const float line_height = ImGui::GetFontSize() + 4.f;
	const float padding = 8.f;

	float max_width = ImGui::CalcTextSize("spectators").x;

	for (auto& name : spectators)
	{
		const float width = ImGui::CalcTextSize(name.c_str()).x;

		if (width > max_width)
			max_width = width;
	}

	const float panel_width = max_width + padding * 2.f;
	const float panel_height = line_height * static_cast<float>(spectators.size() + 1) + padding;

	const int pos = std::clamp(cvars::visuals.spectator_list_pos, 0, 1);
	const ImVec2 start = (pos == 0) ? ImVec2(GImGui->IO.DisplaySize.x - panel_width - 12.f, 90.f) : ImVec2(12.f, 90.f);

	const ImColor color(cvars::visuals.spectator_list_color);

	g_pRenderer->AddRectFilled(start, ImVec2(start.x + panel_width, start.y + panel_height), ImColor(18, 18, 20, 205), 5.f);

	g_pRenderer->AddText(ImVec2(start.x + padding, start.y + padding * 0.5f), color, FontFlags_Shadow, "spectators (%i)", static_cast<int>(spectators.size()));

	float y = start.y + padding * 0.5f + line_height;

	for (auto& name : spectators)
	{
		g_pRenderer->AddText(ImVec2(start.x + padding, y), ImColor(205, 205, 212), FontFlags_Shadow, "%s", name.c_str());

		y += line_height;
	}

	ImGui::PopFont();
}

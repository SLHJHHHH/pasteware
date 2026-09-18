#include "framework.h"

constexpr auto TOGGLE_STATUS_TIME = 4.0;

ToggleStatusData g_ToggleStatus;
std::deque<BulletTracerData> g_BulletTracers;

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

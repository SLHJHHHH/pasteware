#include "framework.h"

static constexpr auto GRENADE_THROW_SPEED = 750.f;
static constexpr auto GRENADE_ELASTICITY = 0.45f;
static constexpr auto GRENADE_FRICTION = 0.65f;
static constexpr auto GRENADE_STEP = 0.02f;

static float GrenadeDetonationTime(int weapon_id)
{
	switch (weapon_id)
	{
	case WEAPON_HEGRENADE:
		return 3.5f;
	case WEAPON_FLASHBANG:
		return 2.0f;
	case WEAPON_SMOKEGRENADE:
		return 1.5f;
	}

	return 3.5f;
}

void CDrawWorld::Overlay()
{
	if (cvars::visuals.grenade_preview)
		GrenadePreview();
}

void CDrawWorld::SimulateTrajectory(const Vector& start, const Vector& velocity, std::deque<Vector>& points, Vector& impact)
{
	const float gravity = max(g_Engine.pfnGetCvarFloat("sv_gravity"), 100.f);

	Vector pos(start), vel(velocity);

	impact = pos;

	for (int i = 0; i < 400; i++)
	{
		Vector next = pos + vel * GRENADE_STEP;

		vel.z -= gravity * GRENADE_STEP;

		pmtrace_t tr;

		g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
		g_Engine.pEventAPI->EV_PlayerTrace(pos, next, PM_NORMAL, -1, &tr);

		pos = tr.endpos;

		if (tr.fraction < 1.f)
		{
			const Vector normal(tr.plane.normal);
			const float dot = vel.x * normal.x + vel.y * normal.y + vel.z * normal.z;

			vel.x -= normal.x * dot * (1.f + GRENADE_ELASTICITY);
			vel.y -= normal.y * dot * (1.f + GRENADE_ELASTICITY);
			vel.z -= normal.z * dot * (1.f + GRENADE_ELASTICITY);

			vel *= GRENADE_FRICTION;

			pos.x += normal.x * 0.5f;
			pos.y += normal.y * 0.5f;
			pos.z += normal.z * 0.5f;

			if (vel.Length() < 40.f)
			{
				points.push_back(pos);
				break;
			}
		}

		points.push_back(pos);
	}

	impact = pos;
}

void CDrawWorld::GrenadePreview()
{
	if (g_Local->m_bIsDead || g_pIGameUI->IsGameUIActive())
		return;

	const float time = static_cast<float>(client_state->time);
	const ImColor color(cvars::visuals.grenade_preview_color);

	while (!m_grenades.empty() && time > m_grenades.front().detonate_time)
		m_grenades.pop_front();

	// trajectory of the currently held grenade
	if (g_Weapon.IsNade() && g_Local->m_bIsConnected)
	{
		float power = 1.f;

		if (g_Weapon->m_flStartThrow > 0.f)
			power = std::clamp(time - g_Weapon->m_flStartThrow, 0.1f, 0.5f) / 0.5f;

		Vector view_angles, direction;

		g_Engine.GetViewAngles(view_angles);
		g_Engine.pfnAngleVectors(view_angles, direction, NULL, NULL);
		direction.Normalize();

		std::deque<Vector> points;
		Vector impact;

		SimulateTrajectory(g_Local->m_vecEyePos, direction * (GRENADE_THROW_SPEED * power), points, impact);

		int index = 0;

		for (auto& point : points)
		{
			if (index++ % 2)
				continue;

			Vector2D screen;

			if (!Game::WorldToScreen(point, screen))
				continue;

			g_pRenderer->AddCircleFilled(ImVec2(screen.x, screen.y), 1.6f, color, 12);
		}
	}

	// thrown grenades: detonation timers
	static int previous_clip = 0;
	static int previous_weapon = -1;

	const bool bThrown = (g_Weapon->m_iWeaponID == previous_weapon && g_Weapon->m_iClip < previous_clip && g_Weapon.IsNade());

	previous_clip = g_Weapon->m_iClip;
	previous_weapon = g_Weapon->m_iWeaponID;

	if (bThrown)
	{
		Vector view_angles, direction;

		g_Engine.GetViewAngles(view_angles);
		g_Engine.pfnAngleVectors(view_angles, direction, NULL, NULL);
		direction.Normalize();

		GrenadeTrailData grenade;

		grenade.type = previous_weapon;
		grenade.detonate_time = time + GrenadeDetonationTime(previous_weapon);

		SimulateTrajectory(g_Local->m_vecEyePos, direction * GRENADE_THROW_SPEED, grenade.points, grenade.origin);
		grenade.detonate_time = time + GrenadeDetonationTime(previous_weapon);

		m_grenades.push_back(grenade);

		while (m_grenades.size() > 6)
			m_grenades.pop_front();
	}

	if (!cvars::visuals.grenade_preview_timer)
		return;

	ImGui::PushFont(g_pFontList[Verdana_17px]);

	for (auto& grenade : m_grenades)
	{
		Vector2D screen;

		if (!Game::WorldToScreen(grenade.origin, screen))
			continue;

		const float remaining = max(grenade.detonate_time - time, 0.f);

		g_pRenderer->AddCircle(ImVec2(screen.x, screen.y), 8.f, color, 24, 1.f);
		g_pRenderer->AddText(ImVec2(screen.x, screen.y - 4.f), color, FontFlags_CenterX | FontFlags_CenterY | FontFlags_Shadow, "%.1f", remaining);
	}

	ImGui::PopFont();
}
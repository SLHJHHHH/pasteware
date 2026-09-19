#include "framework.h"

std::unique_ptr<CRageBot> g_pRageBot;

CRageBot::CRageBot()
{
	m_bFreestanding = false;
	m_bFakeDuckState = false;
	m_bFakeDuckActive = false;
	m_bResolverForceUpdate = false;
	m_flEnemyShotTime = 0.f;
}

CRageBot::~CRageBot()
{

}

void CRageBot::OnEnemyShot()
{
	m_flEnemyShotTime = static_cast<float>(client_state->time);
}

void CRageBot::FakeDuck(usercmd_s* cmd)
{
	m_bFakeDuckActive = false;

	if (!cvars::ragebot.fake_duck)
		return;

	if (!Game::IsConnected() || g_Local->m_bIsDead)
		return;

	if (cvars::ragebot.fake_duck_key.keynum && !m_bFakeDuckState)
		return;

	if (!g_Local->m_bIsOnGround || g_Local->m_bIsOnLadder || g_Local->m_bIsInWater)
		return;

	if (IS_NULLPTR(g_pMiscellaneous) || g_pGlobals->m_nNewCommands < 2)
		return;

	// crouch flicker that is only partially delivered to the server (choked),
	// which keeps the client-side position standing while the model crouches
	if (g_pMiscellaneous->m_iChokedCommands >= std::clamp(cvars::ragebot.fake_duck_choke, 1, g_pGlobals->m_nNewCommands - 1))
		return;

	cmd->buttons |= IN_DUCK;

	Game::SendCommand(false);

	m_bFakeDuckActive = true;
}

void CRageBot::Run(usercmd_s* cmd)
{
	RtlSecureZeroMemory(&m_TargetData, sizeof(m_TargetData));

	if (g_Weapon.IsGun())
		Aimbot(cmd);
}

void CRageBot::FakeLag(usercmd_s* cmd)
{
	if (!IS_NULLPTR(g_pMiscellaneous) && cvars::ragebot.fakelag_enabled)
	{
		if (!g_pGlobals->m_flGaitMovement)
			return;

		if (!cvars::ragebot.fakelag_while_shooting && (cmd->buttons & IN_ATTACK) && g_Weapon.CanAttack())
			return;

		bool bAllow = false;

		if ((cvars::ragebot.fakelag_triggers[0] && g_Local->m_bIsOnGround) ||
			(cvars::ragebot.fakelag_triggers[1] && !g_Local->m_bIsOnGround))
			bAllow = true;

		if (!bAllow)
			return;

		if (cvars::ragebot.fakelag_on_enemy_in_pvs)
		{
			bool bEnemyInPVS = false;

			for (int i = 1; i <= g_Engine.GetMaxClients(); i++)
			{
				if (g_Player[i]->m_bIsLocal)
					continue;

				if (!g_Player[i]->m_bIsConnected)
					continue;

				if (g_Player[i]->m_bIsDead)
					continue;

				if (!g_Player[i]->m_bIsInPVS)
					continue;

				if (!cvars::ragebot.friendly_fire && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
					continue;

				bEnemyInPVS = true;
				break;
			}

			if (!bEnemyInPVS)
				return;
		}

		if (cvars::ragebot.fakelag_on_peek && !m_TargetData.weight && !cvars::ragebot.raim_low_fps_mitigations)
			return;

		if (cvars::ragebot.fakelag_type == 0)
		{
			if (g_pMiscellaneous->m_iChokedCommands < cvars::ragebot.fakelag_choke_limit && g_pMiscellaneous->m_iChokedCommands < g_pGlobals->m_nNewCommands)
				Game::SendCommand(false);
		}
		else if (cvars::ragebot.fakelag_type == 1)
		{
			static const float flDistance = sqrt(LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR);

			float flLength = g_Local->m_flVelocity * g_Local->m_flFrameTime;

			if (flLength < flDistance && flLength)
			{
				int nNeedChoke = (int)(flDistance / flLength);

				if (nNeedChoke > cvars::ragebot.fakelag_choke_limit)
					nNeedChoke = cvars::ragebot.fakelag_choke_limit;
				else if (nNeedChoke > g_pGlobals->m_nNewCommands)
					nNeedChoke = g_pGlobals->m_nNewCommands;

				if (g_pMiscellaneous->m_iChokedCommands < nNeedChoke)
					Game::SendCommand(false);
			}
		}
	}
}

void CRageBot::AntiAimbot(usercmd_s* cmd)
{
	m_QAntiAimbotAngle.Clear();

	if (cvars::ragebot.aa_enabled)
	{
		if (cmd->buttons & IN_USE)
			return;

		if (g_Local->m_bIsOnLadder)
			return;

		if (pmove->waterjumptime)
			return;

		if (g_Weapon.IsNade() && cvars::ragebot.aa_conditions[1])
		{
			if (~cmd->buttons & IN_ATTACK)
			{
				if (g_Weapon->m_flStartThrow > 0.f)
					return;

				if (g_Weapon->m_flReleaseThrow > 0.f)
					return;
			}
		}
		else if (g_Weapon.IsC4())
		{

		}
		else if (g_Weapon.IsKnife())
		{
			if (!cvars::ragebot.aa_conditions[0])
				return;

			if (g_Weapon.CanAttack() && (cmd->buttons & IN_ATTACK || cmd->buttons & IN_ATTACK2))
				return;
		}
		else
		{
			if (g_Weapon.IsNonAttack())
				return;

			if (cmd->buttons & IN_ATTACK && g_Weapon.CanAttack())
				return;
		}

		if (!cvars::ragebot.aa_conditions[2] && g_Weapon->m_iUser3 & PLAYER_FREEZE_TIME_OVER)
			return;

		QAngle QAngles(cmd->viewangles), QNewAngles;

		QAngles.Normalize();

		QNewAngles = QAngles;

		if (cvars::ragebot.aa_at_targets)
		{
			std::deque<int> players;

			for (int i = 1; i <= g_Engine.GetMaxClients(); i++)
			{
				if (g_Player[i]->m_bIsLocal)
					continue;

				if (!g_Player[i]->m_bIsConnected)
					continue;

				if (g_Player[i]->m_bIsDead)
					continue;

				if (!g_Player[i]->m_bIsInPVS)
					continue;

				if (!cvars::ragebot.aa_teammates && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
					continue;

				players.push_back(i);
			}

			if (!players.empty())
			{
				Vector vecSrc(g_Local->m_vecEyePos);

				if (cvars::ragebot.aa_at_targets == 1)
				{
					Vector vecSpreadDir, vecViewAngles;

					g_Engine.GetViewAngles(vecViewAngles);

					g_Engine.pfnAngleVectors(vecViewAngles, vecSpreadDir, NULL, NULL);

					vecSpreadDir.Normalize();

					auto best_player = 0;
					auto best_fov = FLT_MAX;

					for (auto i = 0u; i < players.size(); i++)
					{
						auto fov = vecSpreadDir.AngleBetween(g_Player[players[i]]->m_vecOrigin - vecSrc);

						if (fov < best_fov)
						{
							best_fov = fov;
							best_player = players[i];
						}
					}

					if (best_player)
						Math::VectorAngles(g_Player[best_player]->m_vecOrigin - vecSrc, QNewAngles);
				}
				else if (cvars::ragebot.aa_at_targets == 2)
				{
					Vector vecOrigin;

					for (auto i = 0u; i < players.size(); i++)
						vecOrigin += g_Player[players[i]]->m_vecOrigin;

					vecOrigin /= players.size();

					Math::VectorAngles(vecOrigin - vecSrc, QNewAngles);
				}
			}
		}

		static bool bJitter = false;

		if (!g_pMiscellaneous->m_iChokedCommands)
			bJitter = !bJitter;

		// freestanding: pick the anti-aim side with the most space behind the local player
		bool bSide = cvars::ragebot.aa_side;

		if (cvars::ragebot.aa_freestanding && cvars::ragebot.aa_freestanding_distance > 0.f)
		{
			bool bActive = true;

			if (cvars::ragebot.aa_freestanding_mode == 1)
				bActive = m_bFreestanding;
			else if (cvars::ragebot.aa_freestanding_mode == 2)
				bActive = m_bFreestanding || !cvars::ragebot.aa_freestanding_key.keynum;

			if (bActive)
			{
				Vector view_angles, forward, right;

				g_Engine.GetViewAngles(view_angles);
				g_Engine.pfnAngleVectors(view_angles, forward, right, NULL);

				Vector origin(g_Local->m_vecEyePos);
			Vector left_end = origin - right * cvars::ragebot.aa_freestanding_distance;
			Vector right_end = origin + right * cvars::ragebot.aa_freestanding_distance;

			pmtrace_t tr;

			g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
			g_Engine.pEventAPI->EV_PlayerTrace(origin, left_end, PM_NORMAL, -1, &tr);
			const float flLeftDistance = tr.fraction;
			g_Engine.pEventAPI->EV_PlayerTrace(origin, right_end, PM_NORMAL, -1, &tr);
			const float flRightDistance = tr.fraction;

			if (flLeftDistance != flRightDistance)
				bSide = flLeftDistance > flRightDistance;
			}
		}

		if (!g_pGlobals->m_flGaitMovement)
		{
			if (cvars::ragebot.aa_side_switch_when_take_damage)
			{
				static auto prev_health = g_Local->m_iHealth;

				if (prev_health != g_Local->m_iHealth)
				{
					prev_health = g_Local->m_iHealth;

					cvars::ragebot.aa_side = !cvars::ragebot.aa_side;
				}
			}

			if (cvars::ragebot.aa_stand_pitch == 1)
				QNewAngles.x = 89.f;
			else if (cvars::ragebot.aa_stand_pitch == 2)
				QNewAngles.x = -89.f;
			else if (cvars::ragebot.aa_stand_pitch == 3)
				QNewAngles.x = 180.f;
			else if (cvars::ragebot.aa_stand_pitch == 4)
				QNewAngles.x = -179.99999f;
			else if (cvars::ragebot.aa_stand_pitch == 5)
				QNewAngles.x = bJitter ? 89.f : -89.f;
			else if (cvars::ragebot.aa_stand_pitch == 6)
				QNewAngles.x = bJitter ? 180.f : -179.99999f;

			if (cvars::ragebot.aa_stand_yaw == 1)
				QNewAngles.y += 180.f;
			else if (cvars::ragebot.aa_stand_yaw == 2)
				QNewAngles.y += bJitter ? 90.f : -90.f;
			else if (cvars::ragebot.aa_stand_yaw == 3)
				QNewAngles.y += bJitter ? 140.f : -140.f;
			else if (cvars::ragebot.aa_stand_yaw == 4)
				QNewAngles.y += cvars::ragebot.aa_stand_yaw_static;
			else if (cvars::ragebot.aa_stand_yaw == 5)
				QNewAngles.y = QAngles.y;

			if (cvars::ragebot.aa_stand_desync)
			{
				if (cvars::ragebot.aa_stand_yaw == 6)
					QNewAngles.y += bSide ? -90.f : 90.f;

				if (!g_pMiscellaneous->m_iChokedCommands)
				{
					float flDesyncAngle = 0.f;

					if (cvars::ragebot.aa_stand_desync == 1)
						flDesyncAngle = bSide ? -120.f : 120.f;
					else if (cvars::ragebot.aa_stand_desync == 2)
						flDesyncAngle = bSide ? -170.f : 170.f;

					if (cvars::ragebot.aa_stand_yaw == 2 || cvars::ragebot.aa_stand_yaw == 3)
					{
						float flGaitYawDifference = fmod(g_Local->m_flGaitYaw - g_pGlobals->m_flGaitYaw, 360.f);

						if (flGaitYawDifference < -180)
							flGaitYawDifference += 360;
						else if (flGaitYawDifference > 180)
							flGaitYawDifference -= 360;

						if (flGaitYawDifference < -abs(flDesyncAngle) || flGaitYawDifference > abs(flDesyncAngle))
							flDesyncAngle = -flDesyncAngle;
					}

					float flAngle = -QNewAngles.y + g_pGlobals->m_flGaitYaw + flDesyncAngle;

					QNewAngles.y += flAngle;

					Game::SendCommand(false);
				}
			}
		}
		else
		{
			if (cvars::ragebot.aa_move_pitch == 1)
				QNewAngles.x = 89.f;
			else if (cvars::ragebot.aa_move_pitch == 2)
				QNewAngles.x = -89.f;
			else if (cvars::ragebot.aa_move_pitch == 3)
				QNewAngles.x = 180.f;
			else if (cvars::ragebot.aa_move_pitch == 4)
				QNewAngles.x = -179.99999f;
			else if (cvars::ragebot.aa_move_pitch == 5)
				QNewAngles.x = bJitter ? 89.f : -89.f;
			else if (cvars::ragebot.aa_move_pitch == 6)
				QNewAngles.x = bJitter ? 180.f : -179.99999f;

			if (cvars::ragebot.aa_move_yaw == 1)
				QNewAngles.y += 180.f;
			else if (cvars::ragebot.aa_move_yaw == 2)
				QNewAngles.y = QAngles.y;
			else if (cvars::ragebot.aa_move_yaw == 3)
			{
				QNewAngles.y += 180.f;

				if (!g_pMiscellaneous->m_iChokedCommands)
				{
					float flAngle = -QNewAngles.y + g_pGlobals->m_flGaitYaw + (bJitter ? 120.f : -120.f);

					QNewAngles.y += flAngle;

					Game::SendCommand(false);
				}
			}
		}

		// on shot: hide the real side for a short time after an enemy fires
		if (cvars::ragebot.aa_on_shot && cvars::ragebot.aa_on_shot_time > 0.f && m_flEnemyShotTime > 0.f &&
			static_cast<float>(client_state->time) - m_flEnemyShotTime <= cvars::ragebot.aa_on_shot_time)
		{
			QNewAngles.x = bSide ? -89.f : 89.f;
			QNewAngles.y += bSide ? 120.f : -120.f;
			QNewAngles.z = bSide ? 60.f : -60.f;
		}

		if (cvars::ragebot.aa_roll == 1)
			QNewAngles.z = bJitter ? 50.f : -50.f;
		else if (cvars::ragebot.aa_roll == 2)
			QNewAngles.z = bJitter ? 90.f : -90.f;
		else if (cvars::ragebot.aa_roll == 3)
		{
			QNewAngles.z = bJitter ? 180.f : 0.f;

			if (bJitter)
				QNewAngles.y += 180.f;
		}
		else if (cvars::ragebot.aa_roll == 4)
			QNewAngles.z = cvars::ragebot.aa_roll_static;

		if (g_Local->m_bIsInWater && (QNewAngles.x >= -89.f && QNewAngles.x <= 89.f))
			QNewAngles.x = QAngles.x;

		if (cvars::ragebot.aa_stand_desync && cvars::ragebot.aa_stand_desync_helper && g_Local->m_bIsOnGround && !g_pGlobals->m_flGaitMovement)
		{
			QAngle QTempAngles(0, g_pGlobals->m_flGaitYaw, 0), QTempAngles2(0, g_Local->m_flGaitYaw, 0), QDeltaAngles;

			QTempAngles.Normalize();
			QTempAngles2.Normalize();

			QDeltaAngles = QTempAngles.Delta360(QTempAngles2);

			if (QDeltaAngles.y < 45.f)
				cmd->sidemove = (g_Engine.pfnRandomFloat(0.f, 1.f) > 0.5f ? -pmove->maxspeed : pmove->maxspeed);
		}

		QNewAngles.Normalize();

		if (cvars::ragebot.aa_untrusted_checks)
		{
			if (QNewAngles.x > 89.f)
				QNewAngles.x = 89.f;
			else if (QNewAngles.x < -89.f)
				QNewAngles.x = -89.f;

			if (QNewAngles.z != 0.f)
				QNewAngles.z = 0.f;
		}

		Game::MakeAngle(QNewAngles, cmd);

		m_QAntiAimbotAngle = QNewAngles;
	}
}

void CRageBot::Aimbot(usercmd_s* cmd)
{
	if (cvars::ragebot.raim_key.keynum && !m_bAimState)
		return;

	if (!cvars::weapons[g_Weapon->m_iWeaponID].raim_enabled)
		return;

	float flFPS = 1.f / g_Local->m_flFrameTime;

	if (cvars::ragebot.raim_low_fps_mitigations[0] && flFPS < cvars::ragebot.raim_low_fps_value && (g_Weapon->m_flNextPrimaryAttack > 0.f || g_Weapon->m_flNextSecondaryAttack > 0.f))
		return;

	if (g_Weapon->m_fInReload > 0.f)
		return;

	if (g_Weapon->m_flNextAttack > 0.f)
		return;

	if (!g_Weapon->m_iClip)
		return;

	if (cvars::ragebot.raim_tapping_mode && g_Weapon->m_iShotsFired)
		return;

	if (cvars::ragebot.raim_fov <= 0.f)
		return;

	std::deque<int> hitboxes;

	if (cvars::weapons[g_Weapon->m_iWeaponID].raim_hitboxes[0])
		hitboxes.push_back(HITBOX_HEAD);

	if (cvars::weapons[g_Weapon->m_iWeaponID].raim_hitboxes[1])
		hitboxes.push_back(HITBOX_NECK);

	if (cvars::weapons[g_Weapon->m_iWeaponID].raim_hitboxes[2])
	{
		hitboxes.push_back(HITBOX_LOWER_CHEST);
		hitboxes.push_back(HITBOX_CHEST);
		hitboxes.push_back(HITBOX_UPPER_CHEST);
	}

	if (cvars::weapons[g_Weapon->m_iWeaponID].raim_hitboxes[3])
		hitboxes.push_back(HITBOX_STOMACH);

	if (cvars::weapons[g_Weapon->m_iWeaponID].raim_hitboxes[4])
	{
		hitboxes.push_back(HITBOX_LEFT_UPPER_ARM);
		hitboxes.push_back(HITBOX_LEFT_HAND);
		hitboxes.push_back(HITBOX_LEFT_FOREARM);
		hitboxes.push_back(HITBOX_LEFT_WRIST);
		hitboxes.push_back(HITBOX_RIGHT_UPPER_ARM);
		hitboxes.push_back(HITBOX_RIGHT_HAND);
		hitboxes.push_back(HITBOX_RIGHT_FOREARM);
		hitboxes.push_back(HITBOX_RIGHT_WRIST);
	}

	if (cvars::weapons[g_Weapon->m_iWeaponID].raim_hitboxes[5])
	{
		hitboxes.push_back(HITBOX_LEFT_FOOT);
		hitboxes.push_back(HITBOX_LEFT_CALF);
		hitboxes.push_back(HITBOX_LEFT_THIGH);
		hitboxes.push_back(HITBOX_RIGHT_FOOT);
		hitboxes.push_back(HITBOX_RIGHT_CALF);
		hitboxes.push_back(HITBOX_RIGHT_THIGH);
	}

	if (hitboxes.empty())
		return;

	Vector vecSpreadDir, vecSrc(g_Local->m_vecEyePos), vecAdjustedOrigin;

	{
		QAngle QAngles(cmd->viewangles);

		QAngles[0] += client_state->punchangle[0] * 2.f;
		QAngles[1] += client_state->punchangle[1] * 2.f;

		QAngles.Normalize();

		QAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

		vecSpreadDir.Normalize();
	}

	std::deque<CRageBotTarget> targets;

	for (int i = 1; i <= g_Engine.GetMaxClients(); i++)
	{
		cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!Game::IsValidEntity(pGameEntity))
			continue;

		if (g_Player[i]->m_bIsLocal)
			continue;

		if (!g_Player[i]->m_bIsConnected)
			continue;

		if (g_Player[i]->m_bIsDead)
			continue;

		if (!g_Player[i]->m_bIsInPVS)
			continue;

		if (!cvars::ragebot.friendly_fire && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
			continue;

		if (cvars::ragebot.raim_delayshot[0] || cvars::ragebot.raim_delayshot[1])
		{
			cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(g_Player[i]->m_iEntIndex);

			if (pGameEntity)
			{
				if (cvars::ragebot.raim_delayshot[0])
				{
					Vector vecDelta = pGameEntity->curstate.origin - pGameEntity->prevstate.origin;

					if (vecDelta.LengthSqr() > LAG_COMPENSATION_TELEPORTED_DISTANCE_SQR)
						continue;
				}

				if (cvars::ragebot.raim_delayshot[1])
				{
					if (pGameEntity->curstate.animtime == pGameEntity->prevstate.animtime)
						continue;
				}
			}
		}

		Vector vecTempAdjustedOrigin(g_Player[i]->m_vecOrigin);

		if (!IS_NULLPTR(g_pMiscellaneous) && g_pMiscellaneous->m_bFakeLatencyActive)
			Game::BacktrackPlayer(pGameEntity, -1, vecTempAdjustedOrigin);

		vecTempAdjustedOrigin += Game::PredictPlayer(i);

		CorrectPhysentSolid(i);

		Physent::SetOrigin(i, vecTempAdjustedOrigin);

		for (auto hitbox : hitboxes)
		{
			auto hitgroup = Game::GetHitgroup(hitbox);

			Vector vecHitbox(vecTempAdjustedOrigin + g_Player[i]->m_vecHitbox[hitbox] - g_Player[i]->m_vecOrigin);

			{
				Vector vecAimOrigin = vecHitbox;

				float flFOV = vecSpreadDir.AngleBetween(vecAimOrigin - vecSrc);

				if (flFOV < cvars::ragebot.raim_fov)
				{
					pmtrace_s pmTrace;
					g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
					g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + (vecHitbox - vecSrc) * g_Weapon->m_flDistance, PM_NORMAL, -1, &pmTrace);

					if (g_Engine.pEventAPI->EV_IndexFromTrace(&pmTrace) == g_Player[i]->m_iEntIndex)
					{
						int iDamage = Game::TakeDamage(vecSrc, vecAimOrigin, hitgroup, i);

						if (iDamage)
							targets.push_back(CRageBotTarget{ i, hitbox, -1, iDamage, 0, flFOV, vecAimOrigin, false });
					}
					else if (cvars::weapons[g_Weapon->m_iWeaponID].raim_auto_penetration)
					{
						int iDamage = Game::TakeSimulatedDamage(vecSrc, vecAimOrigin, hitgroup, i);

						if (iDamage && (!cvars::ragebot.raim_safepoint || iDamage >= g_Player[i]->m_iHealth))
							targets.push_back(CRageBotTarget{ i, hitbox, -1, iDamage, 0, flFOV, vecAimOrigin, true });
					}
				}
			}

			if (flFPS < cvars::ragebot.raim_low_fps_value)
			{
				if (cvars::ragebot.raim_low_fps_mitigations[1] && (hitgroup == HITGROUP_LEFTARM || hitgroup == HITGROUP_RIGHTARM))
					continue;
				else if (cvars::ragebot.raim_low_fps_mitigations[2] && (hitgroup == HITGROUP_LEFTLEG || hitgroup == HITGROUP_RIGHTLEG))
					continue;
			}

			{
				float flHitboxScale = GetScaleOfHitbox(hitbox);

				if (flHitboxScale)
				{
					for (auto point = 0; point < HITBOX_POINTS_MAX; point++)
					{
						Vector vecAimOrigin = vecHitbox + ((g_Player[i]->m_vecHitboxPoints[hitbox][point] - g_Player[i]->m_vecHitbox[hitbox]) * (flHitboxScale / 100.f));

						float flFOV = vecSpreadDir.AngleBetween(vecAimOrigin - vecSrc);

						if (flFOV < cvars::ragebot.raim_fov)
						{
							pmtrace_s pmTrace;
							g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
							g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + (vecHitbox - vecSrc) * g_Weapon->m_flDistance, PM_NORMAL, -1, &pmTrace);

							if (g_Engine.pEventAPI->EV_IndexFromTrace(&pmTrace) == g_Player[i]->m_iEntIndex)
							{
								int iDamage = Game::TakeDamage(vecSrc, vecAimOrigin, hitgroup, i);

								if (iDamage)
									targets.push_back(CRageBotTarget{ i, hitbox, point, iDamage, 0, flFOV, vecAimOrigin });
							}
							else if(cvars::weapons[g_Weapon->m_iWeaponID].raim_auto_penetration)
							{
								int iDamage = Game::TakeSimulatedDamage(vecSrc, vecAimOrigin, hitgroup, i);

								if (iDamage && (!cvars::ragebot.raim_safepoint || iDamage >= g_Player[i]->m_iHealth))
									targets.push_back(CRageBotTarget{ i, hitbox, point, iDamage, 0, flFOV, vecAimOrigin, true });
							}
						}
					}
				}
			}
		}
	}

	if (targets.empty())
	{
		if (cvars::ragebot.raim_auto_scope == 2 && g_Weapon.IsSniper() && g_Local->m_bIsScoped)
			cmd->buttons |= IN_ATTACK2;

		return;
	}



	std::deque<CRageBotTarget>::iterator best_target_damage = targets.begin(), best_target_fov = targets.begin();

	auto best_weight = 0;
	auto best_damage = 0;
	auto best_damage_force = false;
	auto best_fov = cvars::ragebot.raim_fov;

	for (auto target = targets.begin(); target != targets.end(); target++)
	{
		// auto force damage: on low health targets only accept shots that will kill
		if (cvars::ragebot.raim_auto_force_damage)
		{
			const int iHealth = g_Player[target->index]->m_iHealth;

			if (iHealth > 0 && iHealth <= cvars::ragebot.raim_auto_force_damage_hp && target->damage < iHealth)
				continue;
		}

		if (cvars::ragebot.raim_target_selection[0])
		{
			if (target->damage >= g_Player[target->index]->m_iHealth)
			{
				best_damage = target->damage;
				best_target_damage = target;
				best_damage_force = true;
				break;
			}
			else if (target->damage > best_damage)
			{
				best_damage = target->damage;
				best_target_damage = target;
			}
		}

		if (cvars::ragebot.raim_target_selection[1])
		{
			if (target->fov < best_fov)
			{
				best_fov = target->fov;
				best_target_fov = target;
			}
		}

		if (cvars::ragebot.raim_target_selection[2])
			target->weight += GetWeightOfHitbox(target->hitbox);

		// safepoint: strongly prefer targets that are visible without penetration
		if (cvars::ragebot.raim_safepoint && target->through_wall)
			target->weight -= 100;
	}

	if (best_damage)
		best_target_damage->weight += cvars::ragebot.raim_target_weight_damage;

	int iForceMin = cvars::weapons[g_Weapon->m_iWeaponID].raim_force_min_damage ? cvars::weapons[g_Weapon->m_iWeaponID].raim_force_min_damage : (g_pRageBot->m_bForceMinDamage ? cvars::ragebot.raim_force_min_damage_value : 0); int iMinDmg = iForceMin ? iForceMin : cvars::weapons[g_Weapon->m_iWeaponID].raim_auto_penetration_min_damage; if (cvars::weapons[g_Weapon->m_iWeaponID].raim_auto_penetration && best_damage < iMinDmg && !best_damage_force)
		return;

	if (best_fov < cvars::ragebot.raim_fov)
		best_target_fov->weight += cvars::ragebot.raim_target_weight_fov;

	for (auto target = targets.begin(); target != targets.end(); target++)
	{
		if (target->weight > best_weight)
		{
			best_weight = target->weight;
			m_TargetData = *target;
		}
	}

	if (!best_weight)
		return;

	// resolver auto: after enough misses on the selected player, force the next resolver yaw option
	if (cvars::ragebot.raim_resolver_auto && g_HitRegister.m_iResolverTarget == m_TargetData.index)
	{
		g_HitRegister.m_iResolverTarget = -1;
		m_bResolverForceUpdate = true;
	}

	if (cvars::weapons[g_Weapon->m_iWeaponID].raim_autostop && g_Local->m_bIsOnGround)
	{
		if (cvars::weapons[g_Weapon->m_iWeaponID].raim_autostop == 1)
		{
			float flMaxSpeed = -1.f;

			switch (g_Weapon->m_iWeaponID)
			{
			case WEAPON_AUG:
			case WEAPON_GALIL:
			case WEAPON_FAMAS:
			case WEAPON_M249:
			case WEAPON_M4A1:
			case WEAPON_AK47:
			case WEAPON_SG552:
				flMaxSpeed = 140.f;
				break;
			case WEAPON_P90:
			case WEAPON_SCOUT:
				flMaxSpeed = 170.f;
				break;
			case WEAPON_P228:
			case WEAPON_ELITE:
			case WEAPON_FIVESEVEN:
			case WEAPON_USP:
			case WEAPON_GLOCK18:
			case WEAPON_DEAGLE:
			case WEAPON_SG550:
			case WEAPON_G3SG1:
				flMaxSpeed = 0.f;
				break;
			case WEAPON_AWP:
				flMaxSpeed = 10.f;
				break;
			}

			if (flMaxSpeed != -1.f)
				Game::ClampSpeed(flMaxSpeed, cmd);
		}
		else if (cvars::weapons[g_Weapon->m_iWeaponID].raim_autostop == 2)
			Game::ClampSpeed(0.f, cmd);

		if (cvars::weapons[g_Weapon->m_iWeaponID].raim_autostop_crouch)
			cmd->buttons |= IN_DUCK;
	}









	assert(m_TargetData.index >= 1 && m_TargetData.index <= MAX_CLIENTS);
	assert(m_TargetData.hitbox > -1 && m_TargetData.hitbox < HITBOX_MAX);
	assert(m_TargetData.point >= -1 && m_TargetData.point < HITBOX_POINTS_MAX);
	assert(m_TargetData.weight >= 0);
	assert(m_TargetData.damage >= 0);
	assert(isfinite(m_TargetData.fov));

	if (cvars::ragebot.raim_auto_scope && g_Weapon.IsSniper() && g_Local->m_iFOV == DEFAULT_FOV)
	{
		cmd->buttons &= ~IN_ATTACK;
		cmd->buttons |= IN_ATTACK2;
		return;
	}

	QAngle QAimAngles, QNewAngles;

	Math::VectorAngles(m_TargetData.origin - vecSrc, QAimAngles);

	QNewAngles = QAimAngles;

	if (cvars::ragebot.raim_remove_recoil)
	{
		QNewAngles[0] -= client_state->punchangle[0] * 2.f;
		QNewAngles[1] -= client_state->punchangle[1] * 2.f;
	}

	g_pNoSpread->GetSpreadOffset(g_Weapon->m_iRandomSeed, 1, QNewAngles, QNewAngles, cvars::ragebot.raim_remove_spread);

	if (g_Weapon.CanAttack())
	{
		if (cvars::ragebot.raim_auto_fire)
			cmd->buttons |= IN_ATTACK;

		if (cmd->buttons & IN_ATTACK)
		{
			Game::MakeAngle(QNewAngles, cmd);

			if (cvars::ragebot.raim_type == 2 && g_pMiscellaneous->m_iChokedCommands < g_pGlobals->m_nNewCommands)
				Game::SendCommand(false);
		}
	}
	else
		cmd->buttons &= ~IN_ATTACK;

	if (cvars::ragebot.raim_type == 0)
		g_Engine.SetViewAngles(QAimAngles);
}

void CRageBot::CorrectPhysentSolid(const int& nPlayerID)
{
	assert(nPlayerID >= 1 && nPlayerID <= MAX_CLIENTS);

	for (int i = 1; i <= client_state->maxclients; i++)
	{
		if (g_Player[i]->m_bIsLocal)
			continue;

		if (!g_Player[i]->m_bIsConnected)
			continue;

		if (g_Player[i]->m_bIsDead)
			continue;

		if (!g_Player[i]->m_bIsInPVS)
			continue;

		if (i == nPlayerID)
		{
			Physent::SetSolid(i, SOLID_BBOX);
			continue;
		}

		if (!cvars::ragebot.friendly_fire && g_Player[i]->m_iTeamNum == g_Local->m_iTeamNum)
			Physent::SetSolid(i, SOLID_BBOX);
		else
			Physent::SetSolid(i, SOLID_NOT);
	}
}

float CRageBot::GetScaleOfHitbox(const int& nHitbox)
{
	assert(nHitbox >= HITBOX_STOMACH && nHitbox < HITBOX_MAX);

	switch (nHitbox)
	{
	case HITBOX_HEAD:
		return cvars::weapons[g_Weapon->m_iWeaponID].raim_head_scale;
	case HITBOX_NECK:
		return cvars::weapons[g_Weapon->m_iWeaponID].raim_neck_scale;
	case HITBOX_UPPER_CHEST:
	case HITBOX_CHEST:
	case HITBOX_LOWER_CHEST:
		return cvars::weapons[g_Weapon->m_iWeaponID].raim_chest_scale;
	case HITBOX_STOMACH:
		return cvars::weapons[g_Weapon->m_iWeaponID].raim_stomach_scale;
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_HAND:
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_WRIST:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_HAND:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_RIGHT_WRIST:
		return cvars::weapons[g_Weapon->m_iWeaponID].raim_arms_scale;
	case HITBOX_LEFT_FOOT:
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_FOOT:
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_THIGH:
		return cvars::weapons[g_Weapon->m_iWeaponID].raim_legs_scale;
	default:
		return 0.f;
	}
}

int CRageBot::GetWeightOfHitbox(const int& nHitbox)
{
	assert(nHitbox >= HITBOX_STOMACH && nHitbox < HITBOX_MAX);

	if (m_bForceBody && cvars::ragebot.raim_force_body_key.keynum)
	{
		if (nHitbox != HITBOX_STOMACH && nHitbox != HITBOX_LOWER_CHEST && nHitbox != HITBOX_CHEST && nHitbox != HITBOX_UPPER_CHEST)
			return 1;
	}

	switch (nHitbox) {
	case HITBOX_HEAD:
		return cvars::ragebot.raim_target_weight_head;
	case HITBOX_NECK:
		return cvars::ragebot.raim_target_weight_neck;
	case HITBOX_LOWER_CHEST:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
		return cvars::ragebot.raim_target_weight_chest;
	case HITBOX_STOMACH:
		return cvars::ragebot.raim_target_weight_stomach;
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_HAND:
	case HITBOX_LEFT_FOREARM:
	case HITBOX_LEFT_WRIST:
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_HAND:
	case HITBOX_RIGHT_FOREARM:
	case HITBOX_RIGHT_WRIST:
		return cvars::ragebot.raim_target_weight_arms;
	case HITBOX_LEFT_FOOT:
	case HITBOX_LEFT_CALF:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_FOOT:
	case HITBOX_RIGHT_CALF:
	case HITBOX_RIGHT_THIGH:
		return cvars::ragebot.raim_target_weight_legs;
	default:
		return 0;
	}
}
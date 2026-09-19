#include "framework.h"

std::unique_ptr<CMiscellaneous> g_pMiscellaneous;

CMiscellaneous::CMiscellaneous()
{
	m_iChokedCommands = 0;
	m_iHudCommands = 0;
	m_bFakeLatencyActive = false;
	m_bPositionAdjustmentActive = false;
	m_iDoubleTapCharge = 0;
	m_bDoubleTapShifting = false;
	m_flLastDoubleTapTime = 0.0;
	m_bAirStuckActive = false;
	m_bFakeLatencyHoldKey = false;
	m_bAutoBlockKey = false;
	m_bAutoBoostKey = false;
	m_bSpeedKey = false;
	m_bAirStuckKey = false;
	m_bDoubleTapKey = false;
	m_iFrameSkipCounter = 0;
	m_flGameSpeedMultiplier = 1.f;
	m_flPositionAdjustmentInterpAmount = 0.f;
}

CMiscellaneous::~CMiscellaneous()
{

}

void CMiscellaneous::NameStealer()
{
	if (!cvars::misc.namestealer)
		return;

	static const std::string english[] = { "A", "a", "E", "e", "O", "o", "X", "x", "C", "c", "B", "K", "H", "P", "p", "T", "M" };
	static const std::string russian[] = { u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�", u8"�" };

	static auto previous_time = client_state->time;

	if (abs(client_state->time - previous_time) > (double)cvars::misc.namestealer_interval)
	{
		previous_time = client_state->time;

		std::deque<std::string> nicknames;

		for (int i = 1; i <= client_state->maxclients; i++)
		{
			if (g_Player[i]->m_ClassId == EClassEntity_BaseLocal)
				continue;

			if (!g_Player[i]->m_bIsConnected)
				continue;

			if (g_Player[i]->m_iTeamNum == TEAM_SPECTATOR || g_Player[i]->m_iTeamNum == TEAM_UNASSIGNED)
				continue;

			if (strlen(g_Player[i]->m_szPrintName))
				nicknames.push_back(g_Player[i]->m_szPrintName);
		}

		while (nicknames.size())
		{
			bool replaced = false;

			int random = g_Engine.pfnRandomLong(0, nicknames.size() - 1);

			assert(random >= 0 && random < nicknames.size());

			std::string nickname = nicknames[random];


			for (size_t j = 0; j < IM_ARRAYSIZE(english); j++)
			{
				auto pos = nickname.find(english[j]);

				if (pos != std::string::npos)
				{
					nickname = nickname.replace(pos, english[j].size(), russian[j]);
					replaced = true;
					break;
				}
			}

			if (!replaced)
			{
				for (size_t j = 0; j < IM_ARRAYSIZE(russian); j++)
				{
					auto pos = nickname.find(russian[j]);

					if (pos != std::string::npos)
					{
						nickname = nickname.replace(pos, russian[j].size(), english[j]);
						replaced = true;
						break;
					}
				}
			}

			if (replaced)
			{
				std::string cmd = "name \"" + nickname + "\"";
				g_Engine.pfnClientCmd(cmd.c_str());
				break;
			}

			nicknames.erase(nicknames.begin() + random);
		}
	}
}

void CMiscellaneous::KillSay(int victim, bool headshot)
{
	if (!cvars::misc.kill_say)
		return;

	if (cvars::misc.kill_say_headshot_only && !headshot)
		return;

	char pszText[128];

	snprintf(pszText, sizeof(pszText), V(cvars::misc.kill_say_text), (victim > 0 && victim <= MAX_CLIENTS) ? g_Player[victim]->m_szPrintName : "enemy");

	std::string cmd = "say \"" + std::string(pszText) + "\"";

	g_Engine.pfnClientCmd(cmd.c_str());
}

void CMiscellaneous::FakeLatency()
{
	m_bFakeLatencyActive = false;

	bool active = cvars::misc.fakelatency;

	if (cvars::misc.fakelatency_hold && cvars::misc.fakelatency_key.keynum)
		active = m_bFakeLatencyHoldKey;

	if (active)
	{
		const int amount_ms = std::clamp(cvars::misc.fakelatency_amount, 0, 1000);
		const auto latency = amount_ms / 1000.0;

		if (latency > 0.0)
		{
			m_bFakeLatencyActive = true;

			Game::SetFakeLatency(latency);
		}
	}
}

bool CMiscellaneous::IsFakeLatencyHoldActive() const
{
	return m_bFakeLatencyHoldKey;
}

void CMiscellaneous::SetFakeLatencyHoldKey(bool down)
{
	m_bFakeLatencyHoldKey = down;
}

void CMiscellaneous::SetAutoBlockKey(bool down)
{
	m_bAutoBlockKey = down;
}

void CMiscellaneous::SetAutoBoostKey(bool down)
{
	m_bAutoBoostKey = down;
}

void CMiscellaneous::SetSpeedKey(bool down)
{
	m_bSpeedKey = down;
}

void CMiscellaneous::SetAirStuckKey(bool down)
{
	m_bAirStuckKey = down;
}

void CMiscellaneous::SetDoubleTapKey(bool down)
{
	m_bDoubleTapKey = down;
}

void CMiscellaneous::AutoBlock(usercmd_s* cmd)
{
	if (!cvars::misc.auto_block || !cvars::misc.auto_block_key.keynum)
		return;

	if (!m_bAutoBlockKey)
		return;

	if (!g_Local->m_bIsConnected || g_Local->m_bIsDead)
		return;

	bool teammate_near = false;

	for (int i = 1; i <= g_Engine.GetMaxClients(); i++)
	{
		if (g_Player[i]->m_bIsLocal)
			continue;

		if (!g_Player[i]->m_bIsConnected || g_Player[i]->m_bIsDead || !g_Player[i]->m_bIsInPVS)
			continue;

		if (g_Player[i]->m_iTeamNum != g_Local->m_iTeamNum)
			continue;

		if (g_Player[i]->m_vecOrigin.Distance(g_Local->m_vecOrigin) < 120.f)
		{
			teammate_near = true;
			break;
		}
	}

	if (teammate_near)
		cmd->buttons |= IN_USE;
}

void CMiscellaneous::AutoBoost(usercmd_s* cmd)
{
	if (!cvars::misc.auto_boost || !cvars::misc.auto_boost_key.keynum)
		return;

	if (!m_bAutoBoostKey)
		return;

	if (!g_Local->m_bIsConnected || g_Local->m_bIsDead)
		return;

	cmd->buttons |= IN_DUCK;
}

void CMiscellaneous::PredictPlayers()
{
	if (cvars::misc.predict_local && g_ClientCvarsMap["cl_lc"])
		g_ClientCvarsMap["cl_lc"]->value = 1.f;

	if (cvars::misc.predict_players && g_ClientCvarsMap["cl_lw"])
		g_ClientCvarsMap["cl_lw"]->value = 1.f;
}

void CMiscellaneous::ApplyClientWeapons()
{
	if (!cvars::misc.client_weapons)
		return;

	if (g_ClientCvarsMap["cl_lw"])
		g_ClientCvarsMap["cl_lw"]->value = 1.f;
}

void CMiscellaneous::FrameSkip()
{
	m_iFrameSkipCounter++;
}

bool CMiscellaneous::ShouldSkipFrame() const
{
	if (!cvars::misc.frame_skip)
		return false;

	const int amount = std::clamp(cvars::misc.frame_skip_amount, 1, 10);

	if (amount <= 1)
		return false;

	return (m_iFrameSkipCounter % (amount + 1)) == 0;
}

void CMiscellaneous::MaximizeOnRespawn()
{
	if (!cvars::misc.maximize_on_respawn)
		return;

	if (!g_pGlobals->m_hWnd)
		return;

	static bool was_dead = true;

	const bool dead = g_Local->m_bIsDead;

	if (was_dead && !dead)
	{
		if (IsIconic(g_pGlobals->m_hWnd))
			ShowWindow(g_pGlobals->m_hWnd, SW_RESTORE);
	}

	was_dead = dead;
}

void CMiscellaneous::GameSpeed()
{
	m_flGameSpeedMultiplier = 1.f;

	if (!cvars::misc.speed_enabled || !m_bSpeedKey)
		return;

	m_flGameSpeedMultiplier = std::clamp(cvars::misc.speed_value, 0.1f, 5.f);
}

float CMiscellaneous::GetGameSpeedMultiplier() const
{
	return m_flGameSpeedMultiplier;
}

void CMiscellaneous::AirStuck(usercmd_s* cmd)
{
	m_bAirStuckActive = false;

	if (!cvars::misc.airstuck || !cvars::misc.airstuck_key.keynum)
		return;

	if (!m_bAirStuckKey)
		return;

	if (g_Local->m_bIsDead || g_Local->m_bIsOnGround || g_Local->m_bIsOnLadder)
		return;

	Game::SendCommand(false);
	m_bAirStuckActive = true;
}

void CMiscellaneous::DoubleTap(usercmd_s* cmd)
{
	m_bDoubleTapShifting = false;

	if (!cvars::misc.doubletap)
		return;

	const bool key_ok = !cvars::misc.doubletap_key.keynum || m_bDoubleTapKey;

	if (!key_ok)
	{
		m_iDoubleTapCharge = 0;
		return;
	}

	if (g_Local->m_bIsDead || !g_Weapon.IsGun() || !g_Weapon.CanAttack())
	{
		m_iDoubleTapCharge = 0;
		return;
	}

	const double now = client_state ? client_state->time : 0.0;

	if (now - m_flLastDoubleTapTime > 0.15)
	{
		if (m_iDoubleTapCharge < std::clamp(cvars::misc.doubletap_shift, 1, 4))
			m_iDoubleTapCharge++;

		m_flLastDoubleTapTime = now;
	}

	if ((cmd->buttons & IN_ATTACK) && m_iDoubleTapCharge > 0)
	{
		if (m_iChokedCommands < g_pGlobals->m_nNewCommands && m_iChokedCommands < m_iDoubleTapCharge)
		{
			Game::SendCommand(false);
			m_bDoubleTapShifting = true;
		}
		else
		{
			m_iDoubleTapCharge = 0;
		}
	}
}

void CMiscellaneous::ChokedCommandsCounter()
{
	static int previous_seq = -1;

	if (client_static->netchan.outgoing_sequence != previous_seq)
	{
		if (client_static->nextcmdtime == FLT_MAX)
			m_iChokedCommands++;
		else if (g_pMiscellaneous->m_iChokedCommands)
			m_iChokedCommands = 0;

		previous_seq = client_static->netchan.outgoing_sequence;
	}


}

void CMiscellaneous::AutoReload(usercmd_s* cmd)
{
	if (cvars::misc.automatic_reload && g_Weapon.IsGun())
	{
		if (cmd->buttons & IN_ATTACK && g_Weapon->m_iClip < 1)
		{
			cmd->buttons &= ~IN_ATTACK;
			cmd->buttons |= IN_RELOAD;
		}
	}
}

void CMiscellaneous::AutoPistol(usercmd_s* cmd)
{
	if (cvars::misc.automatic_pistol && g_Weapon.IsPistol())
	{
		if (cmd->buttons & IN_ATTACK && !g_Weapon.CanAttack())
		{
			cmd->buttons &= ~IN_ATTACK;
		}
	}
}

void CMiscellaneous::RecordHUDCommands(usercmd_s* cmd)
{
	if (client_static->demorecording)
	{
		if (cmd->buttons & IN_ATTACK && (~m_iHudCommands & IN_ATTACK))
		{
			CL_RecordHUDCommand("+attack");
			m_iHudCommands |= IN_ATTACK;
		}
		else if (!(cmd->buttons & IN_ATTACK) && m_iHudCommands & IN_ATTACK)
		{
			CL_RecordHUDCommand("-attack");
			m_iHudCommands &= ~IN_ATTACK;
		}

		if (cmd->buttons & IN_ATTACK2 && (~m_iHudCommands & IN_ATTACK2))
		{
			CL_RecordHUDCommand("+attack2");
			m_iHudCommands |= IN_ATTACK2;
		}
		else if (!(cmd->buttons & IN_ATTACK2) && m_iHudCommands & IN_ATTACK2)
		{
			CL_RecordHUDCommand("-attack2");
			m_iHudCommands &= ~IN_ATTACK2;
		}
	}
}

float CMiscellaneous::GetInterpAmount(const int& lerp)
{
	assert(lerp >= 0 && lerp <= 100);

	const float maxmove = g_Local->m_flFrameTime * 0.05;

	float diff = (lerp / 1000.0) - m_flPositionAdjustmentInterpAmount;

	diff = std::clamp(diff, -maxmove, maxmove);

	const float interp = (m_flPositionAdjustmentInterpAmount + diff);

	return interp;
}
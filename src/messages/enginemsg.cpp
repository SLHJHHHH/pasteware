#include "framework.h"

std::map<svc_commands_e, pfnEngineMessage> g_ClientEngineMsgsMap;

int MSG_SavedReadCount = 0;
int* MSG_ReadCount = nullptr;
int* MSG_CurrentSize = nullptr;
int* MSG_BadRead = nullptr;
sizebuf_t* MSG_Buffer = nullptr;
HL_MSG_ReadByte MSG_ReadByte = nullptr;
HL_MSG_ReadShort MSG_ReadShort = nullptr;
HL_MSG_ReadLong MSG_ReadLong = nullptr;
HL_MSG_ReadFloat MSG_ReadFloat = nullptr;
HL_MSG_ReadString MSG_ReadString = nullptr;
HL_MSG_ReadCoord MSG_ReadCoord = nullptr;
HL_MSG_ReadBitVec3Coord MSG_ReadBitVec3Coord = nullptr;
HL_MSG_ReadBits MSG_ReadBits = nullptr;
HL_MSG_StartBitReading MSG_StartBitReading = nullptr;
HL_MSG_EndBitReading MSG_EndBitReading = nullptr;

bool HookEngineMsg(const svc_commands_e& index, const pfnEngineMessage& pfn)
{
	if (g_ClientEngineMsgsMap[index])
	{
		Utils::TraceLog(V("> %s: arleady hooked svc index %i.\n"), V(__FUNCTION__), index);
		return false;
	}

	auto pClientEngineMsgs = g_pClientEngineMsgs;

	while (pClientEngineMsgs)
	{
		if (pClientEngineMsgs->opcode == index)
		{
			g_ClientEngineMsgsMap[index] = pClientEngineMsgs->pfnParse;
			pClientEngineMsgs->pfnParse = pfn;
			return true;
		}

		pClientEngineMsgs++;
	}

	Utils::TraceLog(V("> %s: failed to hook svc index %i.\n"), V(__FUNCTION__), index);

	return false;
}

bool UnHookEngineMsg(const svc_commands_e& index)
{
	if (!g_ClientEngineMsgsMap[index])
		return true;

	auto pClientEngineMsgs = g_pClientEngineMsgs;

	while (pClientEngineMsgs)
	{
		if (pClientEngineMsgs->opcode == index)
		{
			pClientEngineMsgs->pfnParse = g_ClientEngineMsgsMap[index];
			g_ClientEngineMsgsMap[index] = nullptr;
			return true;
		}

		pClientEngineMsgs++;
	}

	Utils::TraceLog(V("> %s: failed to unhook svc index %i.\n"), V(__FUNCTION__), index);

	return false;
}

void MSG_SaveReadCount()
{
	MSG_SavedReadCount = *MSG_ReadCount;
}

void MSG_RestoreReadCount()
{
	*MSG_ReadCount = MSG_SavedReadCount;
}

void SVC_SendCvarValue()
{
	MSG_SaveReadCount();

	const char* Name = MSG_ReadString();

	if (Name)
	{
		cvar_t* cvar = g_Engine.pfnGetCvarPointer(Name);

		if (cvar)
		{
			const char* spoof = CSandbox::GetSpoofedValue(cvar);

			if (spoof)
			{
				
				static char szSpoof[256];
				char* backup_string = cvar->string;
				float backup_value = cvar->value;

				strncpy_s(szSpoof, spoof, _TRUNCATE);
				cvar->string = szSpoof;
				cvar->value = (float)atof(szSpoof);

				MSG_RestoreReadCount();

				g_ClientEngineMsgsMap[svc_sendcvarvalue]();

				cvar->string = backup_string;
				cvar->value = backup_value;

				return;
			}
		}
	}

	MSG_RestoreReadCount();

	g_ClientEngineMsgsMap[svc_sendcvarvalue]();
}

void SVC_SendCvarValue2()
{
	MSG_SaveReadCount();

	const int RequestID = MSG_ReadLong();
	(void)RequestID;

	const char* Name = MSG_ReadString();

	if (Name && !IS_NULLPTR(g_pSandbox))
	{
		cvar_t* cvar = g_Engine.pfnGetCvarPointer(Name);

		if (cvar)
		{
			const char* spoof = CSandbox::GetSpoofedValue(cvar);

			if (spoof)
			{
				static char szSpoof[256];
				char* backup_string = cvar->string;
				float backup_value = cvar->value;

				strncpy_s(szSpoof, spoof, _TRUNCATE);
				cvar->string = szSpoof;
				cvar->value = (float)atof(szSpoof);

				MSG_RestoreReadCount();

				g_ClientEngineMsgsMap[svc_sendcvarvalue2]();

				cvar->string = backup_string;
				cvar->value = backup_value;

				return;
			}
		}
	}

	MSG_RestoreReadCount();

	g_ClientEngineMsgsMap[svc_sendcvarvalue2]();
}

void SVC_DeltaPacketEntities()
{
	g_ClientEngineMsgsMap[svc_deltapacketentities]();

	if (!g_pGlobals->m_bIsUnloadingLibrary && Game::IsConnected())
	{
		frame_t frame = client_state->frames[client_state->parsecountmod];

		packet_entities_t packet_entities = frame.packet_entities;

		if (packet_entities.num_entities && packet_entities.entities && client_state->model_precache_count)
		{
			static const char t_model[] = "models/player/terror/terror.mdl";
			static const char ct_model[] = "models/player.mdl";

			int modelindex_t = g_Engine.pEventAPI->EV_FindModelIndex(t_model);
			int modelindex_ct = g_Engine.pEventAPI->EV_FindModelIndex(ct_model);

			for (int i = 0; i < packet_entities.num_entities; i++)
			{
				if (packet_entities.entities[i].aiment >= 1 && packet_entities.entities[i].aiment <= MAX_CLIENTS)
				{
					int index = packet_entities.entities[i].aiment;

					if (g_Player[index]->m_bReplaceModel || cvars::misc.replace_models_with_original)
					{
						if (g_Player[index]->m_iTeamNum == TEAM_TERRORIST && modelindex_t)
							packet_entities.entities[i].modelindex = modelindex_t;
						else if(modelindex_ct)
							packet_entities.entities[i].modelindex = modelindex_ct;
					}
				}
			}
		}
	}
}

void SVC_StuffText()
{
	MSG_SaveReadCount();

	const char* pszCommand = MSG_ReadString();

	if (pszCommand && !IS_NULLPTR(g_pSandbox))
	{
		
		
		std::string commands(pszCommand);

		for (auto const& entry : g_ClientCvarsMap)
		{
			const std::string& cvar_name = entry.first;

			size_t pos = commands.find(cvar_name);

			if (pos == std::string::npos)
				continue;

			
			if (pos > 0 && (isalnum((unsigned char)commands[pos - 1]) || commands[pos - 1] == '_'))
				continue;

			size_t after = pos + cvar_name.length();

			if (after < commands.size() && (isalnum((unsigned char)commands[after]) || commands[after] == '_'))
				continue;

			std::string tmp = commands.substr(after);

			if (!tmp.empty() && tmp[0] == ';')
				continue;

			size_t space = tmp.find(' ');

			if (space == std::string::npos)
				continue;

			tmp = tmp.substr(space + 1);

			std::string server_value;
			bool bOpen = false;

			for (size_t i = 0; i < tmp.length(); i++)
			{
				if (tmp[i] == ';' && !bOpen)
					break;

				if (tmp[i] == '"')
					bOpen = !bOpen;
				else if (bOpen)
					server_value.push_back(tmp[i]);
			}

			if (server_value.empty())
			{
				size_t semi = tmp.find(';');

				if (semi != std::string::npos)
					server_value = tmp.substr(0, semi);
				else
					server_value = tmp;
			}

			server_value.erase(std::remove_if(server_value.begin(), server_value.end(), [](char c) { return (c == '\r' || c == '\n'); }), server_value.end());

			size_t lead = 0;

			while (lead < server_value.length() && server_value[lead] == ' ')
				lead++;

			server_value = server_value.substr(lead);

			
			while (!server_value.empty() && server_value.back() == ' ')
				server_value.pop_back();

			if (!server_value.empty() && server_value.length() < 128)
				CSandbox::RememberServerValue(cvar_name.c_str(), server_value.c_str());
		}
	}

	MSG_RestoreReadCount();

	g_ClientEngineMsgsMap[svc_stufftext]();
}

bool HookEngineMessages(void)
{
	if (g_pClientEngineMsgs)
	{
		if (!HookEngineMsg(svc_sendcvarvalue, SVC_SendCvarValue))
			return false;

		if (!HookEngineMsg(svc_sendcvarvalue2, SVC_SendCvarValue2))
			return false;

		if (!HookEngineMsg(svc_deltapacketentities, SVC_DeltaPacketEntities))
			return false;

		if (!HookEngineMsg(svc_stufftext, SVC_StuffText))
			return false;
	}

	return true;
}

void UnHookEngineMessages(void)
{
	if (g_pClientEngineMsgs)
	{
		UnHookEngineMsg(svc_sendcvarvalue);
		UnHookEngineMsg(svc_sendcvarvalue2);
		UnHookEngineMsg(svc_deltapacketentities);
		UnHookEngineMsg(svc_stufftext);
	}
}
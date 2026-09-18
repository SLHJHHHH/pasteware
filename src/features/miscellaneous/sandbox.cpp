#include "framework.h"

std::unique_ptr<CSandbox> g_pSandbox;

CSandbox::CSandbox()
{
	
}

CSandbox::~CSandbox()
{
	
}

std::string CSandbox::GetCvar(std::string name)
{
	if (!cvars[name].empty())
		return cvars[name];
	
	return std::to_string(g_Engine.pfnGetCvarFloat(name.c_str()));
}

void CSandbox::SetupCvar(std::string name, std::string value)
{
	cvars[name] = value;
}

namespace
{
	
	
	std::map<std::string, std::string> g_ServerForced;

	struct CleanCvarEntry { const char* name; const char* value; };

	
	
	const CleanCvarEntry kCleanCvars[] =
	{
		{ "ex_interp", "0.1" },
		{ "cl_updaterate", "20" },
		{ "cl_cmdrate", "30" },
		{ "rate", "2500" },
		{ "cl_rate", "9999" },
		{ "fps_max", "100" },
	};
}

void CSandbox::RememberServerValue(const char* name, const char* value)
{
	if (!name || !*name || !value)
		return;

	g_ServerForced[name] = value;
}

const char* CSandbox::GetSpoofedValue(cvar_t* cvar)
{
	if (!cvar || !cvar->name)
		return nullptr;

	if (!cvars::misc.sandbox_enabled)
		return nullptr;

	auto forced = g_ServerForced.find(cvar->name);

	if (forced != g_ServerForced.end() && !forced->second.empty())
		return forced->second.c_str();

	for (size_t i = 0; i < IM_ARRAYSIZE(kCleanCvars); i++)
	{
		if (!_stricmp(cvar->name, kCleanCvars[i].name))
			return kCleanCvars[i].value;
	}

	return nullptr;
}
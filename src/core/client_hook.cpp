#include "client_hook.h"

#include "minhook.h"
#include "trace.h"
#include "../features/command_manager.h"
#include "../game/entities.h"
#include "../config/config.h"

#include <Psapi.h>
#include <cstring>

namespace
{
    uintptr_t find_client_table(HMODULE module)
    {
        MODULEINFO info{};
        if (!module || !GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info)))
            return 0;

        auto base = static_cast<const unsigned char*>(info.lpBaseOfDll);
        const auto size = static_cast<size_t>(info.SizeOfImage);
        const char text[] = "ScreenFade";
        uintptr_t string_address = 0;

        for (size_t i = 0; i + sizeof(text) <= size; ++i)
        {
            if (std::memcmp(base + i, text, sizeof(text) - 1) == 0)
            {
                string_address = reinterpret_cast<uintptr_t>(base + i);
                break;
            }
        }

        if (!string_address)
            return 0;

        for (size_t i = 0; i + 5 <= size; ++i)
        {
            if (base[i] != 0x68)
                continue;

            uintptr_t value = 0;
            std::memcpy(&value, base + i + 1, sizeof(value));
            if (value == string_address && i + 0x13 + sizeof(void*) <= size)
                return *reinterpret_cast<const uintptr_t*>(base + i + 0x13);
        }

        return 0;
    }
}

ClientHook& ClientHook::instance()
{
    static ClientHook hook;
    return hook;
}

bool ClientHook::install(HMODULE engine)
{
    if (target_)
        return true;

    auto table = find_client_table(engine);
    if (!table)
    {
        Trace::write("client table missing");
        return false;
    }

    auto function = *reinterpret_cast<void**>(table + sizeof(void*) * 13);
    if (!function)
    {
        Trace::write("create move missing");
        return false;
    }

    target_ = function;
    MH_Initialize();
    if (MH_CreateHook(target_, reinterpret_cast<void*>(&hook), reinterpret_cast<void**>(&original_)) != 0 || MH_EnableHook(target_) != 0)
    {
        target_ = nullptr;
        original_ = nullptr;
        Trace::write("create move hook failed");
        return false;
    }

    Trace::value("CL_CreateMove", target_);
    return true;
}

void ClientHook::restore()
{
    if (!target_)
        return;

    MH_DisableHook(target_);
    MH_RemoveHook(target_);
    target_ = nullptr;
    original_ = nullptr;
}

void __cdecl ClientHook::hook(float frametime, usercmd_s* command, int active)
{
    instance().invoke(frametime, command, active);
}

void ClientHook::invoke(float frametime, usercmd_s* command, int active)
{
    static bool logged = false;
    Entities::instance().update();

    if (original_)
        original_(frametime, command, active);

    CommandManager::instance().run(command);

    if (!logged)
    {
        char line[160]{};
        auto& config = Config::instance();
        sprintf_s(line, "command received active=%d cmd=%p bhop=%d aim=%d aa=%d buttons=%d", active, command, config.get_bool("movement.bhop", true), config.get_bool("aim.enabled"), config.get_bool("antiaim.enabled"), command ? command->buttons : 0);
        Trace::write(line);
        logged = true;
    }
}

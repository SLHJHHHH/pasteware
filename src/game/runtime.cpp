#include "runtime.h"

namespace
{
    constexpr const wchar_t* engine_names[] = {L"hw.dll", L"sw.dll"};
    constexpr const wchar_t* filesystem_names[] = {L"filesystem_stdio.dll", L"filesystem_steam.dll"};
}

GameRuntime& GameRuntime::instance()
{
    static GameRuntime runtime;
    return runtime;
}

GameModule GameRuntime::find(const wchar_t* name) const
{
    auto handle = GetModuleHandleW(name);
    return {handle, reinterpret_cast<uintptr_t>(handle)};
}

bool GameRuntime::initialize()
{
    if (initialized_)
        return true;

    for (auto name : engine_names)
    {
        engine_ = find(name);
        if (engine_.handle)
            break;
    }

    for (auto name : filesystem_names)
    {
        filesystem_ = find(name);
        if (filesystem_.handle)
            break;
    }

    client_ = find(L"client.dll");
    vgui_ = find(L"vgui2.dll");
    gameui_ = find(L"GameUI.dll");

    initialized_ = engine_.handle && client_.handle && vgui_.handle;
    return initialized_;
}

void GameRuntime::shutdown()
{
    initialized_ = false;
    engine_ = {};
    client_ = {};
    vgui_ = {};
    gameui_ = {};
    filesystem_ = {};
}

bool GameRuntime::initialized() const
{
    return initialized_;
}

const GameModule& GameRuntime::engine() const
{
    return engine_;
}

const GameModule& GameRuntime::client() const
{
    return client_;
}

const GameModule& GameRuntime::vgui() const
{
    return vgui_;
}

const GameModule& GameRuntime::gameui() const
{
    return gameui_;
}

const GameModule& GameRuntime::filesystem() const
{
    return filesystem_;
}

void* GameRuntime::interface_from(const GameModule& module, const char* name) const
{
    if (!module.handle || !name)
        return nullptr;

    auto factory = reinterpret_cast<GameFactory>(GetProcAddress(module.handle, "CreateInterface"));
    if (!factory)
        return nullptr;

    return factory(name, nullptr);
}

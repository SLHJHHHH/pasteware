#include "interfaces.h"

#include "runtime.h"

GameInterfaces& GameInterfaces::instance()
{
    static GameInterfaces interfaces;
    return interfaces;
}

bool GameInterfaces::initialize()
{
    auto& runtime = GameRuntime::instance();

    surface_ = runtime.interface_from(runtime.vgui(), "VGUI_Surface030");
    panel_ = runtime.interface_from(runtime.vgui(), "VGUI_Panel009");
    engine_vgui_ = runtime.interface_from(runtime.engine(), "VEngineVGui001");
    client_vgui_ = runtime.interface_from(runtime.client(), "ClientVGUI001");
    filesystem_ = runtime.interface_from(runtime.filesystem(), "VFileSystem017");
    console_ = runtime.interface_from(runtime.gameui(), "GameConsole003");

    return surface_ && panel_ && engine_vgui_ && client_vgui_;
}

void GameInterfaces::clear()
{
    surface_ = nullptr;
    panel_ = nullptr;
    engine_vgui_ = nullptr;
    client_vgui_ = nullptr;
    filesystem_ = nullptr;
    console_ = nullptr;
}

void* GameInterfaces::surface() const
{
    return surface_;
}

void* GameInterfaces::panel() const
{
    return panel_;
}

void* GameInterfaces::engine_vgui() const
{
    return engine_vgui_;
}

void* GameInterfaces::client_vgui() const
{
    return client_vgui_;
}

void* GameInterfaces::filesystem() const
{
    return filesystem_;
}

void* GameInterfaces::console() const
{
    return console_;
}

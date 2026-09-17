#include "module.h"

#include "../game/runtime.h"
#include "../game/offsets.h"
#include "../game/interfaces.h"
#include "hook_manager.h"
#include "console.h"
#include "opengl_hook.h"
#include "trace.h"
#include "client_hook.h"
#include "../game/entities.h"
#include "../features/feature_manager.h"
#include "../features/visuals/crosshair.h"
#include "../features/visuals/esp.h"
#include "../features/command_manager.h"
#include "../features/movement/bunnyhop.h"
#include "../features/antiaim/antiaim.h"
#include "../features/aim/aim.h"
#include "../ui/menu.h"
#include "../ui/menu_renderer.h"
#include "../ui/primordial_menu.h"
#include "../bindings/bindings.h"

Module& Module::instance()
{
    static Module module;
    return module;
}

bool Module::start(HMODULE module)
{
    if (running_)
        return false;

    module_ = module;
    Trace::write("runtime initialize");
    if (!GameRuntime::instance().initialize())
    {
        Trace::write("runtime missing modules");
        return false;
    }

    Trace::value("engine", GameRuntime::instance().engine().handle);
    Trace::value("client", GameRuntime::instance().client().handle);
    Trace::value("vgui", GameRuntime::instance().vgui().handle);

    if (!Offsets::instance().initialize())
    {
        Trace::write("offsets failed");
        GameRuntime::instance().shutdown();
        return false;
    }

    GameInterfaces::instance().initialize();
    auto entities_bound = Entities::instance().bind(GameRuntime::instance().client().handle);
    Trace::write(entities_bound ? "entities bound" : "entities missing");

    FeatureManager::instance().add<Crosshair>();
    FeatureManager::instance().add<Esp>();
    FeatureManager::instance().add<MenuRenderer>();
    if (!FeatureManager::instance().initialize())
    {
        Trace::write("features failed");
        GameInterfaces::instance().clear();
        Offsets::instance().clear();
        GameRuntime::instance().shutdown();
        return false;
    }

    CommandManager::instance().add<BunnyHop>();
    CommandManager::instance().add<AntiAim>();
    CommandManager::instance().add<Aim>();
    Menu::instance().initialize();
    Bindings::instance().set(VK_INSERT, BindMenu);
    if (!CommandManager::instance().initialize())
    {
        Trace::write("commands failed");
        FeatureManager::instance().shutdown();
        GameInterfaces::instance().clear();
        Offsets::instance().clear();
        GameRuntime::instance().shutdown();
        return false;
    }

    auto hooked = OpenGLHook::instance().install(GameRuntime::instance().engine().handle);
    Trace::write(hooked ? "opengl hook installed" : "opengl hook missing");
    auto command_hooked = ClientHook::instance().install(GameRuntime::instance().engine().handle);
    Trace::write(command_hooked ? "create move installed" : "create move missing");

    running_ = true;
    return true;
}

void Module::stop()
{
    HookManager::instance().restore_all();
    ClientHook::instance().restore();
    OpenGLHook::instance().restore();
    PrimordialMenu::instance().shutdown();
    FeatureManager::instance().shutdown();
    CommandManager::instance().shutdown();
    GameRuntime::instance().shutdown();
    Offsets::instance().clear();
    GameInterfaces::instance().clear();
    Entities::instance().clear();
    running_ = false;
}

bool Module::running() const
{
    return running_.load();
}

HMODULE Module::handle() const
{
    return module_;
}

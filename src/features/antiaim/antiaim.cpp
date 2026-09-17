#include "antiaim.h"

#include "../../config/config.h"

#include <cmath>

bool AntiAim::initialize()
{
    auto& config = Config::instance();
    config.set_bool("antiaim.enabled", false);
    config.set_float("antiaim.pitch", 89.0f);
    config.set_float("antiaim.yaw", 180.0f);
    return true;
}

void AntiAim::shutdown()
{
}

void AntiAim::run(usercmd_s* command)
{
    auto& config = Config::instance();
    if (!config.get_bool("antiaim.enabled"))
        return;

    command->viewangles.x = config.get_float("antiaim.pitch");
    command->viewangles.y += config.get_float("antiaim.yaw");

    while (command->viewangles.y > 180.0f)
        command->viewangles.y -= 360.0f;

    while (command->viewangles.y < -180.0f)
        command->viewangles.y += 360.0f;
}

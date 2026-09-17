#include "bunnyhop.h"

#include "../../game/entities.h"
#include "../../config/config.h"

bool BunnyHop::initialize()
{
    Config::instance().set_bool("movement.bhop", true);
    Config::instance().set_bool("movement.autostrafe", false);
    return true;
}

void BunnyHop::shutdown()
{
}

void BunnyHop::run(usercmd_s* command)
{
    if (!Config::instance().get_bool("movement.bhop", true))
        return;

    auto local = Entities::instance().local();
    if (!local || !(command->buttons & IN_JUMP))
        return;

    if (!local->curstate.onground)
        command->buttons &= ~IN_JUMP;
}

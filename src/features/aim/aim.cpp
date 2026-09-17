#include "aim.h"

#include "../../config/config.h"
#include "../../game/entities.h"

#include <cmath>

namespace
{
    float angle_delta(float value)
    {
        while (value > 180.0f)
            value -= 360.0f;
        while (value < -180.0f)
            value += 360.0f;
        return value;
    }
}

bool Aim::initialize()
{
    auto& config = Config::instance();
    config.set_bool("aim.enabled", false);
    config.set_float("aim.fov", 12.0f);
    return true;
}

void Aim::shutdown()
{
}

void Aim::run(usercmd_s* command)
{
    auto& config = Config::instance();
    if (!config.get_bool("aim.enabled"))
        return;

    auto local = Entities::instance().local();
    if (!local)
        return;

    cl_entity_s* target = nullptr;
    float best = config.get_float("aim.fov", 12.0f);

    for (int i = 1; i < 65; ++i)
    {
        auto entity = Entities::instance().get(i);
        if (!entity || entity == local || !entity->player)
            continue;

        Vector point = entity->origin;
        point.z += 36.0f;
        Vector delta = point - local->origin;
        float yaw = std::atan2(delta.y, delta.x) * 180.0f / 3.14159265359f;
        float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        float pitch = -std::atan2(delta.z, distance) * 180.0f / 3.14159265359f;
        float fov = std::sqrt(std::pow(angle_delta(pitch - command->viewangles.x), 2.0f) + std::pow(angle_delta(yaw - command->viewangles.y), 2.0f));

        if (fov < best)
        {
            best = fov;
            target = entity;
        }
    }

    if (!target)
        return;

    Vector point = target->origin;
    point.z += 36.0f;
    Vector delta = point - local->origin;
    float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    command->viewangles.x = -std::atan2(delta.z, distance) * 180.0f / 3.14159265359f;
    command->viewangles.y = std::atan2(delta.y, delta.x) * 180.0f / 3.14159265359f;
}

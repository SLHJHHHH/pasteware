#include "esp.h"

#include "../../config/config.h"
#include "../../game/entities.h"
#include "renderer.h"

#include <cmath>

namespace
{
    bool project(const Vector& point, const Vector& origin, const Vector& angles, int width, int height, float& x, float& y)
    {
        constexpr float pi = 3.14159265359f;
        const float pitch = angles.x * pi / 180.0f;
        const float yaw = angles.y * pi / 180.0f;
        const float sp = std::sin(pitch);
        const float cp = std::cos(pitch);
        const float sy = std::sin(yaw);
        const float cy = std::cos(yaw);

        const Vector delta = point - origin;
        const float forward = delta.x * cy + delta.y * sy;
        const float side = -delta.x * sy + delta.y * cy;
        const float up = delta.z * cp - (delta.x * cy + delta.y * sy) * sp;
        if (forward <= 0.01f)
            return false;

        const float scale = static_cast<float>(width) / (2.0f * std::tan(90.0f * pi / 360.0f));
        x = width * 0.5f + side * scale / forward;
        y = height * 0.5f - up * scale / forward;
        return true;
    }
}

bool Esp::initialize()
{
    Config::instance().set_bool("visuals.esp", true);
    return true;
}

void Esp::shutdown()
{
}

void Esp::update()
{
}

void Esp::render()
{
    if (!Config::instance().get_bool("visuals.esp"))
        return;

    auto local = Entities::instance().local();
    if (!local)
        return;

    auto& renderer = Renderer::instance();
    for (int i = 1; i < 65; ++i)
    {
        auto entity = Entities::instance().get(i);
        if (!entity || entity == local || !entity->player)
            continue;

        Vector bottom = entity->origin;
        Vector top = entity->origin;
        top.z += 72.0f;

        float bx, by, tx, ty;
        if (!project(bottom, local->origin, local->angles, renderer.width(), renderer.height(), bx, by))
            continue;
        if (!project(top, local->origin, local->angles, renderer.width(), renderer.height(), tx, ty))
            continue;

        const float h = by - ty;
        if (h <= 1.0f)
            continue;

        const float w = h * 0.45f;
        renderer.rect(tx - w * 0.5f, ty, w, h, {1.0f, 0.2f, 0.2f, 1.0f});
    }
}

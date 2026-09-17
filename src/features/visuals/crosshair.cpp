#include "crosshair.h"

#include "renderer.h"

bool Crosshair::initialize()
{
    return true;
}

void Crosshair::shutdown()
{
}

void Crosshair::update()
{
}

void Crosshair::render()
{
    auto& renderer = Renderer::instance();
    const float x = renderer.width() * 0.5f;
    const float y = renderer.height() * 0.5f;
    const Color color{1.0f, 1.0f, 1.0f, 1.0f};

    renderer.line(x - 5.0f, y, x - 1.0f, y, color, 1.0f);
    renderer.line(x + 1.0f, y, x + 5.0f, y, color, 1.0f);
    renderer.line(x, y - 5.0f, x, y - 1.0f, color, 1.0f);
    renderer.line(x, y + 1.0f, x, y + 5.0f, color, 1.0f);
}

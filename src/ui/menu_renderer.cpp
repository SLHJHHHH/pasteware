#include "menu_renderer.h"

#include "primordial_menu.h"

bool MenuRenderer::initialize()
{
    return true;
}

void MenuRenderer::shutdown()
{
}

void MenuRenderer::update()
{
}

void MenuRenderer::render()
{
    PrimordialMenu::instance().render();
}

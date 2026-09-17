#include "menu.h"

Menu& Menu::instance()
{
    static Menu menu;
    return menu;
}

void Menu::initialize()
{
    tabs_ = {"Visuals", "Movement", "Aim", "AntiAim", "Misc", "Config"};
    selected_ = 0;
    open_ = true;
}

void Menu::toggle()
{
    open_ = !open_;
}

void Menu::set_open(bool value)
{
    open_ = value;
}

void Menu::select(std::size_t index)
{
    if (index < tabs_.size())
        selected_ = index;
}

bool Menu::open() const
{
    return open_;
}

std::size_t Menu::selected() const
{
    return selected_;
}

const std::vector<std::string>& Menu::tabs() const
{
    return tabs_;
}

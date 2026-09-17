#include "bindings.h"

#include "../ui/menu.h"

Bindings& Bindings::instance()
{
    static Bindings bindings;
    return bindings;
}

void Bindings::set(int key, int action)
{
    binds_[key] = action;
}

void Bindings::clear(int key)
{
    binds_.erase(key);
}

bool Bindings::active(int action) const
{
    for (const auto& [key, value] : binds_)
    {
        if (value == action && (GetAsyncKeyState(key) & 0x8000))
            return true;
    }

    return false;
}

void Bindings::update()
{
    for (const auto& [key, action] : binds_)
    {
        bool pressed = (GetAsyncKeyState(key) & 0x8000) != 0;
        bool was_pressed = previous_[key];

        if (pressed && !was_pressed && action == BindMenu)
            Menu::instance().toggle();

        previous_[key] = pressed;
    }
}

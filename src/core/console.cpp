#include "console.h"

#include <Windows.h>

namespace
{
    using Factory = void* (*)(const char*, int*);
}

Console& Console::instance()
{
    static Console console;
    return console;
}

void Console::set(IGameConsole* console)
{
    console_ = console;
}

void Console::print(const char* text) const
{
    if (!text)
        return;

    auto self = const_cast<Console*>(this);
    if (!self->bind())
    {
        self->pending_.emplace_back(text);
        return;
    }

    console_->DPrintf("%s\n", text);
}

void Console::show() const
{
    auto self = const_cast<Console*>(this);
    if (self->bind() && !console_->IsConsoleVisible())
        console_->Activate();
}

bool Console::bind()
{
    if (console_)
        return true;

    auto module = GetModuleHandleW(L"GameUI.dll");
    if (!module)
        return false;

    auto factory = reinterpret_cast<Factory>(GetProcAddress(module, "CreateInterface"));
    if (!factory)
        return false;

    console_ = static_cast<IGameConsole*>(factory("GameConsole003", nullptr));
    return console_ != nullptr;
}

void Console::update()
{
    if (!bind())
        return;

    show();
    for (const auto& text : pending_)
        console_->DPrintf("%s\n", text.c_str());

    pending_.clear();
}

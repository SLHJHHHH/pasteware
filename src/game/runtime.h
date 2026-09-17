#pragma once

#include <Windows.h>
#include <cstdint>

using GameFactory = void* (*)(const char*, int*);

struct GameModule
{
    HMODULE handle = nullptr;
    uintptr_t base = 0;
};

class GameRuntime
{
public:
    static GameRuntime& instance();

    bool initialize();
    void shutdown();
    bool initialized() const;

    const GameModule& engine() const;
    const GameModule& client() const;
    const GameModule& vgui() const;
    const GameModule& gameui() const;
    const GameModule& filesystem() const;

    void* interface_from(const GameModule& module, const char* name) const;

private:
    GameRuntime() = default;
    GameRuntime(const GameRuntime&) = delete;
    GameRuntime& operator=(const GameRuntime&) = delete;

    GameModule find(const wchar_t* name) const;

    GameModule engine_;
    GameModule client_;
    GameModule vgui_;
    GameModule gameui_;
    GameModule filesystem_;
    bool initialized_ = false;
};

#pragma once

#include "types.h"

class GameState
{
public:
    static GameState& instance();

    void begin_frame(usercmd_s* command);
    void end_frame();

    bool connected() const;
    bool in_game() const;
    usercmd_s* command() const;

    void set_connected(bool value);
    void set_in_game(bool value);

private:
    usercmd_s* command_ = nullptr;
    bool connected_ = false;
    bool in_game_ = false;
};

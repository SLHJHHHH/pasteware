#include "state.h"

GameState& GameState::instance()
{
    static GameState state;
    return state;
}

void GameState::begin_frame(usercmd_s* command)
{
    command_ = command;
}

void GameState::end_frame()
{
    command_ = nullptr;
}

bool GameState::connected() const
{
    return connected_;
}

bool GameState::in_game() const
{
    return in_game_;
}

usercmd_s* GameState::command() const
{
    return command_;
}

void GameState::set_connected(bool value)
{
    connected_ = value;
}

void GameState::set_in_game(bool value)
{
    in_game_ = value;
}

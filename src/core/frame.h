#pragma once

#include "../game/state.h"

class FramePipeline
{
public:
    static FramePipeline& instance();

    void begin(usercmd_s* command);
    void end();

private:
    bool active_ = false;
};

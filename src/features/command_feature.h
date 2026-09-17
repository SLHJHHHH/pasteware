#pragma once

#include "../game/types.h"

class CommandFeature
{
public:
    virtual ~CommandFeature() = default;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
    virtual void run(usercmd_s* command) = 0;

    void set_enabled(bool value) { enabled_ = value; }
    bool enabled() const { return enabled_; }

private:
    bool enabled_ = true;
};

#pragma once

#include "../command_feature.h"

class BunnyHop final : public CommandFeature
{
public:
    bool initialize() override;
    void shutdown() override;
    void run(usercmd_s* command) override;
};

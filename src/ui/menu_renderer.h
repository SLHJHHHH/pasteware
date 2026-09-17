#pragma once

#include "../features/feature.h"

class MenuRenderer final : public Feature
{
public:
    bool initialize() override;
    void shutdown() override;
    void update() override;
    void render() override;
};

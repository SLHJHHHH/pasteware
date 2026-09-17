#pragma once

#include "feature.h"

#include <memory>
#include <utility>
#include <vector>

class FeatureManager
{
public:
    static FeatureManager& instance();

    template <typename T, typename... Args>
    T* add(Args&&... args)
    {
        auto feature = std::make_unique<T>(std::forward<Args>(args)...);
        auto result = feature.get();
        features_.push_back(std::move(feature));
        return result;
    }

    bool initialize();
    void shutdown();
    void update();
    void render();

private:
    std::vector<std::unique_ptr<Feature>> features_;
};

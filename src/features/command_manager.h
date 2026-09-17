#pragma once

#include "command_feature.h"

#include <memory>
#include <utility>
#include <vector>

class CommandManager
{
public:
    static CommandManager& instance();

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
    void run(usercmd_s* command);

private:
    std::vector<std::unique_ptr<CommandFeature>> features_;
};

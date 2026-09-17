#include "command_manager.h"

CommandManager& CommandManager::instance()
{
    static CommandManager manager;
    return manager;
}

bool CommandManager::initialize()
{
    for (auto& feature : features_)
    {
        if (!feature->initialize())
        {
            shutdown();
            return false;
        }
    }

    return true;
}

void CommandManager::shutdown()
{
    for (auto it = features_.rbegin(); it != features_.rend(); ++it)
        (*it)->shutdown();
}

void CommandManager::run(usercmd_s* command)
{
    if (!command)
        return;

    for (auto& feature : features_)
    {
        if (feature->enabled())
            feature->run(command);
    }
}

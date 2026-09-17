#include "feature_manager.h"

FeatureManager& FeatureManager::instance()
{
    static FeatureManager manager;
    return manager;
}

bool FeatureManager::initialize()
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

void FeatureManager::shutdown()
{
    for (auto it = features_.rbegin(); it != features_.rend(); ++it)
        (*it)->shutdown();
}

void FeatureManager::update()
{
    for (auto& feature : features_)
    {
        if (feature->enabled())
            feature->update();
    }
}

void FeatureManager::render()
{
    for (auto& feature : features_)
    {
        if (feature->enabled())
            feature->render();
    }
}

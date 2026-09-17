#include "frame.h"

#include "../bindings/bindings.h"
#include "../features/feature_manager.h"
#include "../features/command_manager.h"

FramePipeline& FramePipeline::instance()
{
    static FramePipeline pipeline;
    return pipeline;
}

void FramePipeline::begin(usercmd_s* command)
{
    if (active_)
        return;

    active_ = true;
    GameState::instance().begin_frame(command);
    Bindings::instance().update();
    CommandManager::instance().run(command);
    FeatureManager::instance().update();
}

void FramePipeline::end()
{
    if (!active_)
        return;

    FeatureManager::instance().render();
    GameState::instance().end_frame();
    active_ = false;
}

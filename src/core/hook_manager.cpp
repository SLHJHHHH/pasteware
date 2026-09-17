#include "hook_manager.h"

HookManager& HookManager::instance()
{
    static HookManager manager;
    return manager;
}

VmtHook* HookManager::create(void* object, std::size_t count)
{
    auto hook = std::make_unique<VmtHook>();
    if (!hook->setup(object, count))
        return nullptr;

    auto result = hook.get();
    hooks_.push_back(std::move(hook));
    return result;
}

void HookManager::restore_all()
{
    for (auto it = hooks_.rbegin(); it != hooks_.rend(); ++it)
        (*it)->restore();

    hooks_.clear();
}

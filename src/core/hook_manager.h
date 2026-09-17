#pragma once

#include "vmt_hook.h"

#include <memory>
#include <vector>

class HookManager
{
public:
    static HookManager& instance();

    VmtHook* create(void* object, std::size_t count);
    void restore_all();

private:
    std::vector<std::unique_ptr<VmtHook>> hooks_;
};

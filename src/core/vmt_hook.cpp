#include "vmt_hook.h"

bool VmtHook::setup(void* object, std::size_t count)
{
    if (!object || !count || installed_)
        return false;

    object_ = reinterpret_cast<void**>(object);
    original_ = *reinterpret_cast<void***>(object);
    table_.assign(original_, original_ + count);
    *object_ = table_.data();
    installed_ = true;
    return true;
}

void* VmtHook::hook(std::size_t index, void* function)
{
    if (!installed_ || index >= table_.size() || !function)
        return nullptr;

    auto original = table_[index];
    table_[index] = function;
    return original;
}

void VmtHook::restore()
{
    if (!installed_)
        return;

    *object_ = original_;
    object_ = nullptr;
    original_ = nullptr;
    table_.clear();
    installed_ = false;
}

bool VmtHook::installed() const
{
    return installed_;
}

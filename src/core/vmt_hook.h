#pragma once

#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <vector>

class VmtHook
{
public:
    bool setup(void* object, std::size_t count);
    void* hook(std::size_t index, void* function);
    void restore();
    bool installed() const;

private:
    void** object_ = nullptr;
    void** original_ = nullptr;
    std::vector<void*> table_;
    bool installed_ = false;
};

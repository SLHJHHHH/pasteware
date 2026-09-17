#pragma once

#include <Windows.h>
#include <cstdint>
#include <string>
#include <unordered_map>

class Offsets
{
public:
    static Offsets& instance();

    bool initialize();
    void clear();
    uintptr_t find(HMODULE module, const std::string& pattern) const;
    void set(const std::string& name, uintptr_t address);
    uintptr_t get(const std::string& name) const;

private:
    std::unordered_map<std::string, uintptr_t> values_;
};

#include "offsets.h"

#include "runtime.h"

#include <Psapi.h>
#include <sstream>
#include <vector>

Offsets& Offsets::instance()
{
    static Offsets offsets;
    return offsets;
}

bool Offsets::initialize()
{
    clear();
    return GameRuntime::instance().initialized();
}

void Offsets::clear()
{
    values_.clear();
}

uintptr_t Offsets::find(HMODULE module, const std::string& pattern) const
{
    if (!module)
        return 0;

    MODULEINFO info{};
    if (!GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info)))
        return 0;

    std::vector<int> bytes;
    std::stringstream stream(pattern);
    std::string token;
    while (stream >> token)
        bytes.push_back(token == "?" || token == "??" ? -1 : std::stoi(token, nullptr, 16));

    if (bytes.empty())
        return 0;

    if (info.SizeOfImage < bytes.size())
        return 0;

    auto base = static_cast<const uint8_t*>(info.lpBaseOfDll);
    for (size_t i = 0; i <= info.SizeOfImage - bytes.size(); ++i)
    {
        bool matched = true;
        for (size_t j = 0; j < bytes.size(); ++j)
        {
            if (bytes[j] >= 0 && base[i + j] != bytes[j])
            {
                matched = false;
                break;
            }
        }

        if (matched)
            return reinterpret_cast<uintptr_t>(base + i);
    }

    return 0;
}

void Offsets::set(const std::string& name, uintptr_t address)
{
    values_[name] = address;
}

uintptr_t Offsets::get(const std::string& name) const
{
    auto it = values_.find(name);
    return it == values_.end() ? 0 : it->second;
}

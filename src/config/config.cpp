#include "config.h"

#include <fstream>
#include <cstdlib>

Config& Config::instance()
{
    static Config config;
    return config;
}

bool Config::load(const std::string& path)
{
    std::ifstream file(path);
    if (!file)
        return false;

    values_.clear();
    std::string line;
    while (std::getline(file, line))
    {
        auto split = line.find('=');
        if (split == std::string::npos)
            continue;

        values_[line.substr(0, split)] = line.substr(split + 1);
    }

    return true;
}

bool Config::save(const std::string& path) const
{
    std::ofstream file(path);
    if (!file)
        return false;

    for (const auto& [name, value] : values_)
        file << name << '=' << value << '\n';

    return true;
}

void Config::set(const std::string& name, const std::string& value)
{
    values_[name] = value;
}

void Config::set_bool(const std::string& name, bool value)
{
    set(name, value ? "1" : "0");
}

void Config::set_int(const std::string& name, int value)
{
    set(name, std::to_string(value));
}

void Config::set_float(const std::string& name, float value)
{
    set(name, std::to_string(value));
}

std::string Config::get(const std::string& name, const std::string& fallback) const
{
    auto it = values_.find(name);
    return it == values_.end() ? fallback : it->second;
}

bool Config::get_bool(const std::string& name, bool fallback) const
{
    auto value = get(name);
    if (value.empty())
        return fallback;

    return value == "1" || value == "true";
}

int Config::get_int(const std::string& name, int fallback) const
{
    auto value = get(name);
    if (value.empty())
        return fallback;

    char* end = nullptr;
    auto result = std::strtol(value.c_str(), &end, 10);
    return end == value.c_str() ? fallback : static_cast<int>(result);
}

float Config::get_float(const std::string& name, float fallback) const
{
    auto value = get(name);
    if (value.empty())
        return fallback;

    char* end = nullptr;
    auto result = std::strtof(value.c_str(), &end);
    return end == value.c_str() ? fallback : result;
}

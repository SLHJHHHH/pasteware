#pragma once

#include <string>
#include <unordered_map>

class Config
{
public:
    static Config& instance();

    bool load(const std::string& path);
    bool save(const std::string& path) const;

    void set(const std::string& name, const std::string& value);
    void set_bool(const std::string& name, bool value);
    void set_int(const std::string& name, int value);
    void set_float(const std::string& name, float value);
    std::string get(const std::string& name, const std::string& fallback = {}) const;
    bool get_bool(const std::string& name, bool fallback = false) const;
    int get_int(const std::string& name, int fallback = 0) const;
    float get_float(const std::string& name, float fallback = 0.0f) const;

private:
    std::unordered_map<std::string, std::string> values_;
};

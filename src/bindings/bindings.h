#pragma once

#include <Windows.h>
#include <unordered_map>

enum BindAction
{
    BindMenu = 1
};

class Bindings
{
public:
    static Bindings& instance();

    void set(int key, int action);
    void clear(int key);
    bool active(int action) const;
    void update();

private:
    std::unordered_map<int, int> binds_;
    std::unordered_map<int, bool> previous_;
};

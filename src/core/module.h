#pragma once

#include <Windows.h>
#include <atomic>

class Module
{
public:
    static Module& instance();

    bool start(HMODULE module);
    void stop();
    bool running() const;
    HMODULE handle() const;

private:
    Module() = default;
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;

    HMODULE module_ = nullptr;
    std::atomic_bool running_ = false;
};

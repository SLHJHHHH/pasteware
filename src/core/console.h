#pragma once

#include <string>
#include <vector>

class IGameConsole
{
public:
    virtual void Activate() = 0;
    virtual void Initialize() = 0;
    virtual void Hide() = 0;
    virtual void Clear() = 0;
    virtual bool IsConsoleVisible() = 0;
    virtual void Printf(const char* format, ...) = 0;
    virtual void DPrintf(const char* format, ...) = 0;
};

class Console
{
public:
    static Console& instance();

    void set(IGameConsole* console);
    void print(const char* text) const;
    void show() const;
    void update();

private:
    bool bind();

    IGameConsole* console_ = nullptr;
    std::vector<std::string> pending_;
};

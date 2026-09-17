#pragma once

#include <Windows.h>

namespace Trace
{
    void open(HMODULE module);
    void write(const char* text);
    void value(const char* name, const void* address);
}

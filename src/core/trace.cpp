#include "trace.h"

#include <cstdio>

namespace
{
    HANDLE file = INVALID_HANDLE_VALUE;
}

void Trace::open(HMODULE module)
{
    char path[MAX_PATH]{};
    GetModuleFileNameA(module, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (!slash)
        return;

    strcpy_s(slash + 1, MAX_PATH - static_cast<size_t>(slash + 1 - path), "pasteware.log");
    file = CreateFileA(path, GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
}

void Trace::write(const char* text)
{
    if (file == INVALID_HANDLE_VALUE)
        return;

    DWORD written = 0;
    WriteFile(file, text, static_cast<DWORD>(strlen(text)), &written, nullptr);
    WriteFile(file, "\r\n", 2, &written, nullptr);
    FlushFileBuffers(file);
}

void Trace::value(const char* name, const void* address)
{
    char line[128]{};
    sprintf_s(line, "%s: %p", name, address);
    write(line);
}

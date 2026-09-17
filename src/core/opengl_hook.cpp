#include "opengl_hook.h"
#include "trace.h"
#include "minhook.h"

#include "../features/feature_manager.h"
#include "../features/visuals/renderer.h"
#include "../game/entities.h"

#include <cstring>
#include <gl/GL.h>

OpenGLHook& OpenGLHook::instance()
{
    static OpenGLHook hook;
    return hook;
}

bool OpenGLHook::install(HMODULE)
{
    if (target_)
        return true;

    auto opengl = GetModuleHandleA("opengl32.dll");
    if (!opengl)
        opengl = LoadLibraryA("opengl32.dll");

    target_ = opengl ? reinterpret_cast<void*>(GetProcAddress(opengl, "wglSwapBuffers")) : nullptr;
    if (!target_)
        return false;

    MH_Initialize();
    if (MH_CreateHook(target_, reinterpret_cast<void*>(&hook), reinterpret_cast<void**>(&original_)) != 0)
    {
        target_ = nullptr;
        return false;
    }

    if (MH_EnableHook(target_) != 0)
    {
        MH_RemoveHook(target_);
        target_ = nullptr;
        original_ = nullptr;
        return false;
    }

    Trace::value("wglSwapBuffers", target_);
    return true;
}

void OpenGLHook::restore()
{
    if (!target_)
        return;

    MH_DisableHook(target_);
    MH_RemoveHook(target_);
    target_ = nullptr;
    original_ = nullptr;
}

BOOL WINAPI OpenGLHook::hook(HDC hdc)
{
    return instance().invoke(hdc);
}

BOOL OpenGLHook::invoke(HDC hdc)
{
    Entities::instance().update();

    static bool rendered = false;
    if (!rendered)
    {
        Trace::write("first frame");
        rendered = true;
    }

    GLint viewport[4]{};
    glGetIntegerv(GL_VIEWPORT, viewport);
    Renderer::instance().set_size(viewport[2], viewport[3]);
    FeatureManager::instance().render();
    return original_ ? original_(hdc) : FALSE;
}

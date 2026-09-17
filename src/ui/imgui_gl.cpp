#include "imgui_gl.h"

#include <imgui.h>
#include <gl/GL.h>

bool ImGuiGl::initialize(HWND window)
{
    if (initialized_ || !window)
        return initialized_;

    if (!QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&ticks_)) || !QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&time_)))
        return false;

    window_ = window;
    auto& io = ImGui::GetIO();
    io.BackendPlatformName = "pasteware_win32";
    io.BackendRendererName = "pasteware_opengl2";
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Space] = VK_SPACE;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';

    unsigned char* pixels = nullptr;
    int width = 0;
    int height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    glGenTextures(1, &font_);
    glBindTexture(GL_TEXTURE_2D, font_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    io.Fonts->SetTexID(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(font_)));
    initialized_ = true;
    return true;
}

void ImGuiGl::shutdown()
{
    if (!initialized_)
        return;

    if (font_)
        glDeleteTextures(1, &font_);

    font_ = 0;
    window_ = nullptr;
    initialized_ = false;
}

void ImGuiGl::new_frame()
{
    auto& io = ImGui::GetIO();
    RECT rect{};
    GetClientRect(window_, &rect);
    io.DisplaySize = ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));

    INT64 now = 0;
    QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&now));
    io.DeltaTime = static_cast<float>(now - time_) / static_cast<float>(ticks_);
    time_ = now;

    POINT point{};
    GetCursorPos(&point);
    ScreenToClient(window_, &point);
    io.MousePos = ImVec2(static_cast<float>(point.x), static_cast<float>(point.y));
    io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
    io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    io.KeyCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
}

void ImGuiGl::render()
{
    auto* draw = ImGui::GetDrawData();
    const int width = static_cast<int>(draw->DisplaySize.x * draw->FramebufferScale.x);
    const int height = static_cast<int>(draw->DisplaySize.y * draw->FramebufferScale.y);
    if (width <= 0 || height <= 0)
        return;

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT_BIT | GL_SCISSOR_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, draw->DisplaySize.x, draw->DisplaySize.y, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    for (int list_index = 0; list_index < draw->CmdListsCount; ++list_index)
    {
        const ImDrawList* list = draw->CmdLists[list_index];
        const ImDrawVert* vertices = list->VtxBuffer.Data;
        const ImDrawIdx* indices = list->IdxBuffer.Data;

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), &vertices[0].pos);
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), &vertices[0].uv);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), &vertices[0].col);

        for (const auto& command : list->CmdBuffer)
        {
            if (command.UserCallback)
            {
                command.UserCallback(list, &command);
                continue;
            }

            const auto clip = command.ClipRect;
            glScissor(static_cast<int>(clip.x), static_cast<int>(height - clip.w), static_cast<int>(clip.z - clip.x), static_cast<int>(clip.w - clip.y));
            glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(reinterpret_cast<intptr_t>(command.TextureId)));
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(command.ElemCount), sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, indices);
            indices += command.ElemCount;
        }

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

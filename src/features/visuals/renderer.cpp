#include "renderer.h"

#include <gl/GL.h>

Renderer& Renderer::instance()
{
    static Renderer renderer;
    return renderer;
}

void Renderer::set_size(int width, int height)
{
    width_ = width;
    height_ = height;
}

int Renderer::width() const
{
    return width_;
}

int Renderer::height() const
{
    return height_;
}

void Renderer::line(float x1, float y1, float x2, float y2, Color color, float width)
{
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(width);
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
    glPopAttrib();
}

void Renderer::rect(float x, float y, float width, float height, Color color, bool filled)
{
    glPushAttrib(GL_ENABLE_BIT | GL_LINE_BIT | GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glColor4f(color.r, color.g, color.b, color.a);
    glBegin(filled ? GL_QUADS : GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
    glPopAttrib();
}

#pragma once

#include <Windows.h>

struct Color
{
    float r;
    float g;
    float b;
    float a;
};

class Renderer
{
public:
    static Renderer& instance();

    void set_size(int width, int height);
    int width() const;
    int height() const;

    void line(float x1, float y1, float x2, float y2, Color color, float width = 1.0f);
    void rect(float x, float y, float width, float height, Color color, bool filled = false);

private:
    int width_ = 0;
    int height_ = 0;
};

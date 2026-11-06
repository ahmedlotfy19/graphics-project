#pragma once
#include "Renderer2D.h"
#include <string>

enum class Align { Left, Center, Right };

namespace Text
{
    void draw(Renderer2D& r, const std::string& text, float x, float y, float size,
              Color color, Align align = Align::Left);
    void drawShadowed(Renderer2D& r, const std::string& text, float x, float y, float size,
                      Color color, Align align = Align::Left);
}

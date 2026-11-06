#pragma once
// ============================================================================
//  Text  -  built-in pixel font.                         Owner: MEMBER 1
//  size = letter height in pixels. A "\n" in the text starts a new line.
// ============================================================================
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

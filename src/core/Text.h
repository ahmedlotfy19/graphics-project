#pragma once
// ============================================================================
//  Text  -  built-in 5x7 pixel font drawn with Renderer2D rectangles.
//  Owner: Member 1 (Core). No font files or extra libraries needed.
//
//      Text::draw(r, "WAVE 1", 640, 400, 32, Colors::White, Align::Center);
//
//  * size = height of a capital letter in virtual pixels.
//  * Lowercase letters are drawn as capitals.
//  * Supports printable ASCII (space .. ~) and '\n' for new lines.
// ============================================================================
#include "Renderer2D.h"
#include <string>

enum class Align { Left, Center, Right };

namespace Text
{
    void  draw(Renderer2D& r, const std::string& text, float x, float y, float size,
               Color color, Align align = Align::Left);
    // Same, with a dark drop shadow behind it (more readable over busy backgrounds)
    void  drawShadowed(Renderer2D& r, const std::string& text, float x, float y, float size,
                       Color color, Align align = Align::Left, Color shadow = Color(0, 0, 0, 0.7f));
    float width(const std::string& text, float size);    // width of the widest line
    float lineHeight(float size);
}

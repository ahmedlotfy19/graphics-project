#pragma once
// ============================================================================
//  Environment  -  background, lanes ground, home base, 5+ props.  Owner: MEMBER 4
//  Draw order used by Game:
//      drawBackground -> Lanes::draw -> drawHomeBase -> (characters...) -> drawForeground
// ============================================================================
class Renderer2D;

namespace Environment
{
    void drawBackground(Renderer2D& r);
    void drawHomeBase(Renderer2D& r, float time);
    void drawForeground(Renderer2D& r, float time);
}

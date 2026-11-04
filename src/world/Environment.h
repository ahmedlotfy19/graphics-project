#pragma once
// ============================================================================
//  Environment  -  background, lanes ground, home base, 5+ props.  Owner: MEMBER 4
//  Placeholder right now. Draw order used by Game:
//      drawBackground -> Lanes::draw -> drawHomeBase -> (characters...) -> drawForeground
// ============================================================================

class Renderer2D;

namespace Environment
{
    void drawBackground(Renderer2D& r, float time);   // sky, far scenery, lane ground
    void drawHomeBase(Renderer2D& r, float time);     // x in [0, HOME_BASE_RIGHT]
    void drawForeground(Renderer2D& r, float time);   // anything drawn over characters (fog, etc.)
}

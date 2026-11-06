#pragma once
// ============================================================================
//  Cutscene  -  the 10-20 s animated intro.         Owner: MEMBER 4
//  Must show at least 1 player + 1 zombie, animated, with a short dialogue
//  (subtitles OK, but not text only), and explain what is being defended.
// ============================================================================
class Renderer2D;

namespace Cutscene
{
    void  reset();
    void  update(float dt);
    void  draw(Renderer2D& r);
    bool  finished();
}

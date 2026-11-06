#pragma once
// ============================================================================
//  Cutscene  -  the 10-20 s animated intro.         Owner: MEMBER 4
//  17 s intro: a survivor and zombies talk and fight in front of the Safe House,
//  then the survivor runs to the lanes. Uses the real player and zombie models.
// ============================================================================
class Renderer2D;

namespace Cutscene
{
    void  reset();
    void  update(float dt);
    void  draw(Renderer2D& r);
    bool  finished();
}

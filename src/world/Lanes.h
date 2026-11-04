#pragma once
// ============================================================================
//  Lanes  -  hover highlight + click-to-select lanes.    Owner: MEMBER 4
//  Spec: hovering highlights the whole lane; left click toggles selection;
//  multiple lanes can be selected. Players (W/S jump/duck) read `selected`.
//  Check Input::mousePressed() - it returns false if a card/button already
//  consumed this click.
// ============================================================================
#include "../Config.h"

class Renderer2D;
struct World;

struct LaneState
{
    int  hovered = -1;
    bool selected[NUM_LANES] = {};
};

namespace Lanes
{
    void update(World& w);
    void draw(Renderer2D& r, const World& w);
}

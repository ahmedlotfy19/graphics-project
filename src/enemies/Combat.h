#pragma once
// ============================================================================
//  Combat  -  who hits whom.                             Owner: MEMBER 3
//  Moves every projectile and applies every hit: axe, club, bolts, spit, skulls.
// ============================================================================

struct World;

namespace Combat
{
    void update(World& w, float dt);
}

#pragma once
// ============================================================================
//  Combat  -  collisions, "both stop and fight", projectile hits,
//             High/Low dodging, damage values.        Owner: MEMBER 3
//  Coordinate with Member 2: use Players::damage(), Players::isProtected()
//  and Player::pose (Jumping avoids Low, Ducking avoids High).
// ============================================================================
struct World;

namespace Combat
{
    void update(World& w, float dt);
}

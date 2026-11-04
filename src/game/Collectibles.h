#pragma once
// ============================================================================
//  Collectibles  -  coins dropped by zombies.   Owner: Member 1
//  Enemies::damage() calls Collectibles::spawn() when a zombie dies.
//  A coin is collected when any player in the same lane touches it.
// ============================================================================

class Renderer2D;
struct World;

namespace Collectibles
{
    void spawn(World& w, int lane, float x, int value);
    void update(World& w, float dt);
    void draw(Renderer2D& r, const World& w);
}

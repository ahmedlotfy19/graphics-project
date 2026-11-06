#pragma once
// ============================================================================
//  Collectibles  -  coins dropped by zombies.            Owner: MEMBER 1
//  spawn() is called by Enemies::damage when a zombie dies.
// ============================================================================

class Renderer2D;
struct World;

namespace Collectibles
{
    void spawn(World& w, int lane, float x, int value);
    void update(World& w, float dt);
    void draw(Renderer2D& r, const World& w);
}

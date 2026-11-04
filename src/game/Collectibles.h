#pragma once

class Renderer2D;
struct World;

namespace Collectibles
{
    void spawn(World& w, int lane, float x, int value);
    void update(World& w, float dt);
    void draw(Renderer2D& r, const World& w);
}

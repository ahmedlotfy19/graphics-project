#include "Collectibles.h"
#include "World.h"
#include "../core/Renderer2D.h"
#include "../core/UI.h"
#include "../core/Audio.h"

#include <cmath>

namespace
{
    const float kGravity     = -900.0f;
    const float kPickupRange = 30.0f;
    const float kRadius      = 13.0f;
}

namespace Collectibles
{
    void spawn(World& w, int lane, float x, int value)
    {
        Collectible c;
        c.lane = lane;
        c.x = x;
        c.y = 40.0f;
        c.vy = 320.0f;
        c.value = value;
        w.collectibles.push_back(c);
    }

    void update(World& w, float dt)
    {
        for (Collectible& c : w.collectibles)
        {
            if (!c.alive) continue;
            c.age += dt;

            if (c.y > 0.0f || c.vy > 0.0f)
            {
                c.vy += kGravity * dt;
                c.y += c.vy * dt;
                if (c.y <= 0.0f)
                {
                    c.y = 0.0f;
                    c.vy = (std::fabs(c.vy) > 120.0f) ? -c.vy * 0.35f : 0.0f;
                }
            }

            for (const Player& p : w.players)
            {
                if (!p.alive || p.lane != c.lane) continue;
                if (std::fabs(p.x - c.x) < p.halfWidth + kPickupRange)
                {
                    c.alive = false;
                    w.coins += c.value;
                    Audio::play(Sfx::Coin);
                    break;
                }
            }
        }
    }

    void draw(Renderer2D& r, const World& w)
    {
        for (const Collectible& c : w.collectibles)
        {
            if (!c.alive) continue;
            float ground = laneGroundY(c.lane);
            float bob = (c.y == 0.0f) ? 3.0f * std::sin(c.age * 4.0f) : 0.0f;
            float cy = ground + kRadius + 4.0f + c.y + bob;

            float shadow = 1.0f / (1.0f + c.y * 0.01f);
            r.ellipse(c.x, ground + 2, kRadius * shadow, 4 * shadow, Color(0, 0, 0, 0.35f), 16);

            float pulse = 0.5f + 0.5f * std::sin(c.age * 5.0f);
            r.circle(c.x, cy, kRadius + 6 + pulse * 3, Colors::rgb(255, 220, 90, 50), 20);
            UI::coinIcon(r, c.x, cy, kRadius, c.age * 4.0f);
        }
    }
}

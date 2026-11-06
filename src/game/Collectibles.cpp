#include "Collectibles.h"
#include "World.h"
#include "../core/Renderer2D.h"
#include "../core/UI.h"
#include "../core/Audio.h"

#include <cmath>
#include <cstdlib>

const float GRAVITY = -1100;
const float BOUNCE = 0.45f;
const float FRICTION = 0.6f;
const float COIN_SIZE = 13;

static float randomFloat(float a, float b)
{
    return a + (b - a) * (rand() / (float)RAND_MAX);
}

namespace Collectibles
{
    void spawn(World& w, int lane, float x, int value)
    {
        Collectible c;
        c.lane = lane;
        c.x = x;
        c.y = 50;
        c.vx = randomFloat(20, 110);
        c.vy = randomFloat(330, 420);
        c.value = value;
        w.collectibles.push_back(c);
    }

    void update(World& w, float dt)
    {
        for (Collectible& c : w.collectibles)
        {
            c.age += dt;
            c.squash -= dt * 5;
            if (c.squash < 0) c.squash = 0;

            if (!c.resting)
            {
                c.vy += GRAVITY * dt;
                c.x += c.vx * dt;
                c.y += c.vy * dt;
                c.spin += (6 + fabs(c.vx) * 0.05f) * dt;

                float left = HOME_BASE_RIGHT + COIN_SIZE;
                float right = FIELD_RIGHT - COIN_SIZE;
                if (c.x < left)  { c.x = left;  c.vx = -c.vx * BOUNCE; }
                if (c.x > right) { c.x = right; c.vx = -c.vx * BOUNCE; }

                if (c.y <= 0)
                {
                    c.y = 0;
                    c.squash = std::fmin(1.0f, fabs(c.vy) / 400);

                    if (fabs(c.vy) > 90)
                    {
                        c.vy = -c.vy * BOUNCE;
                        c.vx *= FRICTION;
                    }
                    else
                    {
                        c.vy = 0;
                        c.vx = 0;
                        c.resting = true;
                    }
                }
            }
            else
            {
                c.spin += 3 * dt;
            }

            for (const Player& p : w.players)
            {
                if (p.alive && p.lane == c.lane && fabs(p.x - c.x) < p.halfWidth + COIN_SIZE + 10
                    && c.y < p.y + p.height)
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
            float ground = laneGroundY(c.lane);
            float bob = c.resting ? 3 * sin(c.age * 4) + 3 : 0;
            float y = ground + COIN_SIZE + 2 + c.y + bob;

            float shadow = 1 / (1 + c.y * 0.012f);
            r.ellipse(c.x, ground + 2, COIN_SIZE * shadow, 4 * shadow, Color(0, 0, 0, 0.35f), 16);

            if (c.resting)
                r.circle(c.x, y, COIN_SIZE + 7, Colors::rgb(255, 220, 90, 50), 20);

            r.push();
            r.translate(c.x, y - COIN_SIZE);
            r.scale(1 + c.squash * 0.3f, 1 - c.squash * 0.35f);
            UI::coinIcon(r, 0, COIN_SIZE, COIN_SIZE, c.spin);
            r.pop();
        }
    }
}

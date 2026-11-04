// ============================================================================
//  PLACEHOLDER - Member 2 replaces this file.
//  What exists: spawning, walking right, stopping when touching a zombie,
//  damage + death. Boxes instead of models.
//  TODO (Member 2):
//    [ ] Real, distinct models for Melee / Ranged / Defense (not just colour)
//    [ ] Weapon animation in the melee player's hand
//    [ ] Ranged projectile (spawn into w.projectiles with fromPlayer = true)
//    [ ] Jump (W/Up) and duck (S/Down) for players in SELECTED lanes
//        (w.lanes.selected[lane]); defense players ignore it
//    [ ] Defense-protection rule in isProtected()
//    [ ] Red damage tint: r.setDamageTint(1 - hp/maxHp) while drawing
// ============================================================================
#include "Players.h"
#include "../game/World.h"
#include "../core/Renderer2D.h"
#include "../core/Text.h"
#include "../core/Audio.h"

#include <cmath>

namespace
{
    const float kWalkSpeed = 45.0f;
}

namespace Players
{
    int maxHpFor(PlayerType t)
    {
        switch (t)
        {
        case PlayerType::Defense: return DEFENSE_PLAYER_HP;
        case PlayerType::Ranged:  return RANGED_PLAYER_HP;
        default:                  return MELEE_PLAYER_HP;
        }
    }

    void spawn(World& w, PlayerType t, int lane)
    {
        Player p;
        p.type = t;
        p.lane = lane;
        p.x = PLAYER_SPAWN_X;
        p.hp = p.maxHp = maxHpFor(t);
        if (t == PlayerType::Defense) { p.halfWidth = 28.0f; p.height = 80.0f; }
        w.players.push_back(p);
        Audio::play(Sfx::Place);
    }

    void update(World& w, float dt)
    {
        for (Player& p : w.players)
        {
            if (!p.alive) continue;
            p.anim += dt;

            p.engaged = false;
            for (const Enemy& e : w.enemies)
                if (e.alive && e.lane == p.lane && std::fabs(e.x - p.x) < p.halfWidth + e.halfWidth)
                    p.engaged = true;

            if (p.type != PlayerType::Defense && !p.engaged)
                p.x = std::fmin(p.x + kWalkSpeed * dt, FIELD_RIGHT - p.halfWidth);
        }
    }

    void draw(Renderer2D& r, const World&, const Player& p)
    {
        float gy = laneGroundY(p.lane) + p.y;
        Color c = p.type == PlayerType::Melee  ? Colors::rgb(60, 110, 200)
                : p.type == PlayerType::Ranged ? Colors::rgb(60, 170, 90)
                                               : Colors::rgb(150, 150, 160);
        r.push();
        r.setDamageTint(1.0f - (float)p.hp / p.maxHp);
        r.rect(p.x - p.halfWidth, gy, p.halfWidth * 2, p.height, c);
        r.rectOutline(p.x - p.halfWidth, gy, p.halfWidth * 2, p.height, 2, Colors::Black);
        const char* letter = p.type == PlayerType::Melee ? "M" : p.type == PlayerType::Ranged ? "R" : "D";
        Text::draw(r, letter, p.x, gy + p.height * 0.5f - 10, 20, Colors::White, Align::Center);
        r.pop();
    }

    void drawProjectile(Renderer2D& r, const Projectile& pr)
    {
        r.circle(pr.x, pr.y, 5, Colors::Yellow);
    }

    void damage(World&, Player& p, int amount)
    {
        if (!p.alive) return;
        p.hp -= amount;
        Audio::play(Sfx::Hit);
        if (p.hp <= 0)
        {
            p.hp = 0;
            p.alive = false;
            Audio::play(Sfx::PlayerDeath);
        }
    }

    bool isProtected(const World&, const Player&)
    {
        return false;   // TODO Member 2
    }
}

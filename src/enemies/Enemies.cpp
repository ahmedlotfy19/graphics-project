// ============================================================================
//  PLACEHOLDER - Member 3 replaces this file.
//  What exists: spawning, walking left, stopping when touching a player,
//  damage, death + coin drop. Boxes instead of models.
//  TODO (Member 3):
//    [ ] Real, distinct zombie models for the 3 types (not just colour)
//    [ ] Upper-ranged: throws attacks that come from ABOVE (AttackHeight::High)
//    [ ] Lower-ranged: attacks that come from BELOW (AttackHeight::Low)
//    [ ] Melee: RUNS (faster) and hits for 2
//    [ ] Attack animations + red damage tint
// ============================================================================
#include "Enemies.h"
#include "../game/World.h"
#include "../game/Collectibles.h"
#include "../core/Renderer2D.h"
#include "../core/Text.h"
#include "../core/Audio.h"

#include <cmath>

namespace
{
    float speedFor(EnemyType t) { return t == EnemyType::Melee ? 55.0f : 28.0f; }
}

namespace Enemies
{
    int maxHpFor(EnemyType t)
    {
        switch (t)
        {
        case EnemyType::Melee:       return HARD_ENEMY_HP;
        case EnemyType::LowerRanged: return MEDIUM_ENEMY_HP;
        default:                     return EASY_ENEMY_HP;
        }
    }

    void spawn(World& w, EnemyType t, int lane)
    {
        Enemy e;
        e.type = t;
        e.lane = lane;
        e.x = ENEMY_SPAWN_X;
        e.hp = e.maxHp = maxHpFor(t);
        w.enemies.push_back(e);
    }

    void update(World& w, float dt)
    {
        for (Enemy& e : w.enemies)
        {
            if (!e.alive) continue;
            e.anim += dt;
            e.engaged = false;
            for (const Player& p : w.players)
                if (p.alive && p.lane == e.lane && std::fabs(e.x - p.x) < p.halfWidth + e.halfWidth)
                    e.engaged = true;
            if (!e.engaged)
                e.x -= speedFor(e.type) * dt;
        }
    }

    void draw(Renderer2D& r, const World&, const Enemy& e)
    {
        float gy = laneGroundY(e.lane);
        Color c = e.type == EnemyType::UpperRanged ? Colors::rgb(110, 150, 80)
                : e.type == EnemyType::LowerRanged ? Colors::rgb(140, 120, 70)
                                                   : Colors::rgb(120, 60, 60);
        r.push();
        r.setDamageTint(1.0f - (float)e.hp / e.maxHp);
        r.rect(e.x - e.halfWidth, gy, e.halfWidth * 2, e.height, c);
        r.rectOutline(e.x - e.halfWidth, gy, e.halfWidth * 2, e.height, 2, Colors::Black);
        const char* letter = e.type == EnemyType::UpperRanged ? "U" : e.type == EnemyType::LowerRanged ? "L" : "H";
        Text::draw(r, letter, e.x, gy + e.height * 0.5f - 10, 20, Colors::White, Align::Center);
        r.pop();
    }

    void drawProjectile(Renderer2D& r, const Projectile& pr)
    {
        r.circle(pr.x, pr.y, 6, Colors::rgb(120, 200, 60));
    }

    void damage(World& w, Enemy& e, int amount)
    {
        if (!e.alive) return;
        e.hp -= amount;
        Audio::play(Sfx::Hit);
        if (e.hp <= 0)
        {
            e.hp = 0;
            e.alive = false;
            Audio::play(Sfx::EnemyDeath);
            Collectibles::spawn(w, e.lane, e.x, COIN_DROP_VALUE);
        }
    }
}

// ============================================================================
//  PLACEHOLDER - Member 3 replaces this file.
//  Right now: touching player/zombie pairs trade 1 hit per second.
//  TODO (Member 3): projectiles (move, hit, dodge via Player::pose),
//  real attack rates, melee zombie 2 dmg, defense protection via
//  Players::isProtected(), defense players absorbing hits.
// ============================================================================
#include "Combat.h"
#include "../game/World.h"

#include <cmath>

namespace Combat
{
    void update(World& w, float dt)
    {
        for (Player& p : w.players)
        {
            if (!p.alive) continue;
            p.attackTimer -= dt;
            for (Enemy& e : w.enemies)
            {
                if (!e.alive || e.lane != p.lane) continue;
                if (std::fabs(e.x - p.x) >= p.halfWidth + e.halfWidth) continue;

                if (p.attackTimer <= 0.0f && p.type != PlayerType::Defense)
                {
                    Enemies::damage(w, e, PLAYER_DAMAGE);
                    p.attackTimer = 1.0f;
                }
                e.attackTimer -= dt;
                if (e.attackTimer <= 0.0f)
                {
                    Players::damage(w, p, e.type == EnemyType::Melee ? HARD_ENEMY_DAMAGE : ENEMY_DAMAGE);
                    e.attackTimer = 1.2f;
                }
                break;
            }
        }

        for (Projectile& pr : w.projectiles)
        {
            pr.age += dt;
            pr.x += pr.vx * dt;
            pr.y += pr.vy * dt;
            if (pr.x < -50 || pr.x > VIRTUAL_W + 50) pr.alive = false;
        }
    }
}

#include "Combat.h"
#include "../game/World.h"

#include <cmath>

namespace
{
    const float kSpitRange   = 440.0f;
    const float kSkullRange  = 520.0f;
    const float kImpactTime  = 0.35f;
    const float kMeleeReach  = 8.0f;
    const float kBruteReach  = 14.0f;
    const float kMeleePeriod = 1.0f;

    void startImpact(Projectile& pr)
    {
        pr.kind |= ENEMY_PROJ_IMPACT;
        pr.age = 0.0f;
        pr.vx = pr.vy = 0.0f;
    }

    Player* nearestPlayer(World& w, const Enemy& e, float reach)
    {
        Player* best = nullptr;
        float bestD = 1e9f;
        for (Player& p : w.players)
        {
            if (!p.alive || p.lane != e.lane) continue;
            float d = std::fabs(p.x - e.x);
            if (d < p.halfWidth + e.halfWidth + reach && d < bestD) { best = &p; bestD = d; }
        }
        return best;
    }

    Enemy* nearestEnemy(World& w, const Player& p, float reach)
    {
        Enemy* best = nullptr;
        float bestD = 1e9f;
        for (Enemy& e : w.enemies)
        {
            if (!e.alive || e.lane != p.lane) continue;
            float d = std::fabs(p.x - e.x);
            if (d < p.halfWidth + e.halfWidth + reach && d < bestD) { best = &e; bestD = d; }
        }
        return best;
    }

    bool dodges(const Player& p, AttackHeight h)
    {
        if (p.type == PlayerType::Defense) return false;
        if (h == AttackHeight::High && p.pose == Pose::Ducking) return true;
        if (h == AttackHeight::Low && p.pose == Pose::Jumping) return true;
        return false;
    }

    void updateEnemyProjectile(World& w, Projectile& pr, float dt)
    {
        int kind = pr.kind & 0xFF;
        float ground = laneGroundY(pr.lane);
        pr.x += pr.vx * dt;

        float range;
        if (kind == ENEMY_PROJ_SPIT)
        {
            pr.y = ground + 76.0f + 3.0f * std::sin(pr.age * 14.0f);
            pr.vy = 42.0f * std::cos(pr.age * 14.0f);
            range = kSpitRange;
        }
        else
        {
            pr.y = ground + 9.0f;
            range = kSkullRange;
        }

        if (pr.age * std::fabs(pr.vx) > range || pr.x < HOME_BASE_RIGHT - 10.0f)
        {
            startImpact(pr);
            return;
        }

        Player* target = nullptr;
        for (Player& p : w.players)
        {
            if (!p.alive || p.lane != pr.lane) continue;
            if (std::fabs(p.x - pr.x) >= p.halfWidth + 6.0f) continue;
            if (dodges(p, pr.height)) continue;
            if (!target || p.x > target->x) target = &p;
        }
        if (target)
        {
            Players::damage(w, *target, pr.damage);
            startImpact(pr);
        }
    }
}

namespace Combat
{
    void update(World& w, float dt)
    {
        for (Enemy& e : w.enemies)
        {
            if (!e.alive || !e.strikeNow) continue;
            e.strikeNow = false;
            if (Player* p = nearestPlayer(w, e, kBruteReach))
                Players::damage(w, *p, HARD_ENEMY_DAMAGE);
        }

        for (Player& p : w.players)
        {
            if (!p.alive || p.type != PlayerType::Melee) continue;
            p.attackTimer -= dt;
            if (p.attackTimer > 0.0f) continue;
            if (Enemy* e = nearestEnemy(w, p, kMeleeReach))
            {
                Enemies::damage(w, *e, PLAYER_DAMAGE);
                p.attackTimer = kMeleePeriod;
            }
            else p.attackTimer = 0.0f;
        }

        for (Projectile& pr : w.projectiles)
        {
            if (!pr.alive) continue;
            pr.age += dt;

            if (pr.fromPlayer)
            {
                pr.x += pr.vx * dt;
                pr.y += pr.vy * dt;
                if (pr.x < -50.0f || pr.x > VIRTUAL_W + 50.0f) pr.alive = false;
                continue;
            }

            if (pr.kind & ENEMY_PROJ_IMPACT)
            {
                if (pr.age > kImpactTime) pr.alive = false;
                continue;
            }
            updateEnemyProjectile(w, pr, dt);
        }
    }
}

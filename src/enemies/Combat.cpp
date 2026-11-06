#include "Combat.h"
#include "../game/World.h"

#include <cmath>

static bool touching(const Player& p, const Enemy& e, float extra)
{
    return p.alive && e.alive && p.lane == e.lane && fabs(p.x - e.x) < p.halfWidth + e.halfWidth + extra;
}

static bool dodged(const Player& p, AttackHeight height)
{
    if (p.type == PlayerType::Defense) return false;
    if (height == AttackHeight::High && p.pose == Pose::Ducking) return true;
    if (height == AttackHeight::Low && p.pose == Pose::Jumping) return true;
    return false;
}

static void bruteAttacks(World& w)
{
    for (Enemy& e : w.enemies)
    {
        if (!e.strikeNow) continue;
        e.strikeNow = false;

        Player* target = nullptr;
        for (Player& p : w.players)
            if (touching(p, e, 14) && (!target || fabs(p.x - e.x) < fabs(target->x - e.x)))
                target = &p;

        if (target)
            Players::damage(w, *target, HARD_ENEMY_DAMAGE);
    }
}

static void meleePlayerAttacks(World& w, float dt)
{
    for (Player& p : w.players)
    {
        if (!p.alive || p.type != PlayerType::Melee) continue;

        p.attackTimer -= dt;
        if (p.attackTimer > 0) continue;

        Enemy* target = nullptr;
        for (Enemy& e : w.enemies)
            if (touching(p, e, 8) && (!target || fabs(p.x - e.x) < fabs(p.x - target->x)))
                target = &e;

        if (target)
        {
            Enemies::damage(w, *target, PLAYER_DAMAGE);
            p.attackTimer = 1.0f;
        }
        else
        {
            p.attackTimer = 0;
        }
    }
}

static void moveProjectiles(World& w, float dt)
{
    for (Projectile& pr : w.projectiles)
    {
        if (!pr.alive) continue;
        pr.age += dt;
        pr.x += pr.vx * dt;
        pr.y += pr.vy * dt;

        if (pr.x < -50 || pr.x > VIRTUAL_W + 50)
            pr.alive = false;

        if (pr.fromPlayer) continue;

        if (pr.kind == SPIT)
            pr.y = laneGroundY(pr.lane) + 76 + 3 * sin(pr.age * 14);

        float range = pr.kind == SPIT ? 300.0f : 520.0f;
        if (pr.age * fabs(pr.vx) > range || pr.x < HOME_BASE_RIGHT)
        {
            pr.alive = false;
            continue;
        }

        Player* target = nullptr;
        for (Player& p : w.players)
        {
            if (!p.alive || p.lane != pr.lane) continue;
            if (fabs(p.x - pr.x) > p.halfWidth + 6) continue;
            if (dodged(p, pr.height)) continue;
            if (!target || p.x > target->x) target = &p;
        }

        if (target)
        {
            Players::damage(w, *target, pr.damage);
            pr.alive = false;
        }
    }
}

namespace Combat
{
    void update(World& w, float dt)
    {
        bruteAttacks(w);
        meleePlayerAttacks(w, dt);
        moveProjectiles(w, dt);
    }
}

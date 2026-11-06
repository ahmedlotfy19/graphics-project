#pragma once
// ============================================================================
//  Projectile + Collectible structs.                     Owner: MEMBER 1
//  Projectile: crossbow bolt (fromPlayer), spit or skull (kind = SPIT / SKULL).
//  Collectible: a coin that bounces on its lane until a player walks into it.
// ============================================================================

enum class AttackHeight { High, Low, Mid };

struct Projectile
{
    bool  fromPlayer = true;
    int   lane = 0;
    float x = 0, y = 0;
    float vx = 0;
    int   damage = 1;
    AttackHeight height = AttackHeight::Mid;
    int   kind = 0;
    float age = 0;
    bool  alive = true;
};

struct Collectible
{
    int   lane = 0;
    float x = 0;
    float y = 0;
    float vx = 0;
    float vy = 0;
    float spin = 0;
    float squash = 0;
    bool  resting = false;
    int   value = 0;
    float age = 0;
    bool  alive = true;
};

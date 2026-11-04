#pragma once
// ============================================================================
//  Projectile + Collectible  -  shared data.  Owner: Member 1.
//  Members 2 & 3 may ADD fields (tell the team), never rename existing ones.
// ============================================================================

// Where an attack travels. Spec 3.2/3.3:
//   High  -> comes from above, player must DUCK to avoid it   (upper-ranged enemy)
//   Low   -> comes from below, player must JUMP to avoid it   (lower-ranged enemy)
//   Mid   -> normal straight shot / melee (player projectiles, melee hits)
enum class AttackHeight { High, Low, Mid };

struct Projectile
{
    bool  fromPlayer = true;   // true = shot by a player (hits enemies only, never players)
    int   lane = 0;
    float x = 0.0f, y = 0.0f;  // y = absolute virtual y (for drawing)
    float vx = 0.0f, vy = 0.0f;
    int   damage = 1;
    AttackHeight height = AttackHeight::Mid;
    int   kind = 0;            // free for the owner: which visual/weapon this is
    float age = 0.0f;          // seconds since fired (for animation)
    bool  alive = true;
};

struct Collectible
{
    int   lane = 0;
    float x = 0.0f;
    float y = 0.0f;            // current height above the lane ground (pops up then lands)
    float vy = 0.0f;
    int   value = 0;
    float age = 0.0f;
    bool  alive = true;
};

#pragma once
// ============================================================================
//  Enemies  -  the 3 zombie types.             Owner: MEMBER 3
//
//  The current .cpp is a PLACEHOLDER (plain boxes, walking). Replace with the
//  real models, animations and High/Low/melee attacks.
//  Fields marked REQUIRED are read by other members' code - keep their names.
// ============================================================================
#include "../Config.h"

class Renderer2D;
struct World;
struct Projectile;

enum class EnemyType
{
    UpperRanged = 0,   // Easy   - attacks from ABOVE (player ducks), 5 hp
    LowerRanged = 1,   // Medium - attacks from BELOW (player jumps), 5 hp
    Melee       = 2,   // Hard   - runs in, close range, 2 dmg, 7 hp
    Count       = 3
};

struct Enemy
{
    // ---- REQUIRED (used by Core / Players / Combat) ----
    EnemyType type = EnemyType::UpperRanged;
    int   lane = 0;
    float x = ENEMY_SPAWN_X;
    int   hp = 1, maxHp = 1;
    float halfWidth = 24.0f;
    float height = 95.0f;      // model height; health bar is drawn above this
    bool  engaged = false;
    bool  alive = true;        // set false to remove (Core erases dead entries)

    // ---- Member 3's own fields ----
    float attackTimer = 0.0f;
    float anim = 0.0f;
};

namespace Enemies
{
    int  maxHpFor(EnemyType t);
    void spawn(World& w, EnemyType t, int lane);
    void update(World& w, float dt);
    void draw(Renderer2D& r, const World& w, const Enemy& e);
    void drawProjectile(Renderer2D& r, const Projectile& pr);   // projectiles with fromPlayer == false

    // Applies damage; at 0 hp marks dead, plays sound and drops a collectible
    // via Collectibles::spawn(). Called by Combat.
    void damage(World& w, Enemy& e, int amount);
}

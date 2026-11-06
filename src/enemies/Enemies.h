#pragma once
// ============================================================================
//  Enemies  -  the 3 zombie types.             Owner: MEMBER 3
//
//   UpperRanged (easy)  : Spitter     - lobs acid that comes down from ABOVE (duck)
//   LowerRanged (medium): Bone Roller - bowls skulls along the GROUND (jump)
//   Melee       (hard)  : Brute       - armoured, runs in, nail club, 2 damage
//
//  Fields marked REQUIRED are read by other members' code - keep their names.
// ============================================================================
#include "../Config.h"

class Renderer2D;
struct World;
struct Projectile;

enum class EnemyType
{
    UpperRanged = 0,
    LowerRanged = 1,
    Melee       = 2,
    Count       = 3
};

constexpr int ENEMY_PROJ_SPIT   = 0;
constexpr int ENEMY_PROJ_SKULL  = 1;
constexpr int ENEMY_PROJ_IMPACT = 0x100;

struct Enemy
{
    // ---- REQUIRED (used by Core / Players / Combat) ----
    EnemyType type = EnemyType::UpperRanged;
    int   lane = 0;
    float x = ENEMY_SPAWN_X;
    int   hp = 1, maxHp = 1;
    float halfWidth = 24.0f;
    float height = 95.0f;
    bool  engaged = false;
    bool  alive = true;

    // ---- Member 3's own fields ----
    float attackTimer = 0.0f;
    float attackAnim = 10.0f;
    float anim = 0.0f;
    float spawnT = 0.0f;
    float hitFlash = 0.0f;
    bool  strikeNow = false;
};

namespace Enemies
{
    int   maxHpFor(EnemyType t);
    float attackInterval(EnemyType t);
    void  spawn(World& w, EnemyType t, int lane);
    void  update(World& w, float dt);
    void  draw(Renderer2D& r, const World& w, const Enemy& e);
    void  drawProjectile(Renderer2D& r, const Projectile& pr);

    void  damage(World& w, Enemy& e, int amount);

    // Draws one zombie without needing a World (for the cutscene). Faces LEFT.
    //   attack : -1 = no attack pose, 0..1 = progress through one attack
    //   charge : 0..1 how close the next attack is (spitter sac / brute wind-up)
    void  drawModel(Renderer2D& r, EnemyType t, float x, float groundY, float anim,
                    bool walking = true, float attack = -1.0f, float charge = 0.0f,
                    float damage = 0.0f);
}

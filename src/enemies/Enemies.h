#pragma once
// ============================================================================
//  Enemies  -  the 3 zombie types.                       Owner: MEMBER 3
//  UpperRanged : Spitter, spits acid at head height (duck)
//  LowerRanged : Bone Roller, rolls skulls along the ground (jump)
//  Melee       : Brute, runs fast and hits with a club for 2
// ============================================================================
#include "../Config.h"

class Renderer2D;
struct World;
struct Projectile;

enum class EnemyType { UpperRanged = 0, LowerRanged = 1, Melee = 2 };

const int SPIT = 0;
const int SKULL = 1;

struct Enemy
{
    EnemyType type = EnemyType::UpperRanged;
    int   lane = 0;
    float x = ENEMY_SPAWN_X;
    int   hp = 1, maxHp = 1;
    float halfWidth = 24;
    float height = 95;
    bool  engaged = false;
    bool  alive = true;

    float anim = 0;
    float attackTimer = 0;
    float attackAnim = 10;
    bool  strikeNow = false;
};

namespace Enemies
{
    void spawn(World& w, EnemyType t, int lane);
    void update(World& w, float dt);
    void draw(Renderer2D& r, const World& w, const Enemy& e);
    void drawProjectile(Renderer2D& r, const Projectile& pr);
    void damage(World& w, Enemy& e, int amount);

    void drawModel(Renderer2D& r, EnemyType t, float x, float groundY, float anim,
                   bool walking = true, float attack = -1, float damage = 0);
}

#pragma once
// ============================================================================
//  Players  -  the 3 player types.            Owner: MEMBER 2
//
//   Melee   : survivor with a fire axe (swings on every attack)
//   Ranged  : ranger with a crossbow  (fires an animated bolt)
//   Defense : riot guard with a big shield (never moves, protects players behind it)
//
//  Fields marked REQUIRED are read by other members' code - keep their names.
// ============================================================================
#include "../Config.h"

class Renderer2D;
struct World;
struct Projectile;

enum class PlayerType { Melee = 0, Ranged = 1, Defense = 2, Count = 3 };
enum class Pose { Normal, Jumping, Ducking };

struct Player
{
    // ---- REQUIRED (used by Core / Enemies / Combat) ----
    PlayerType type = PlayerType::Melee;
    int   lane = 0;
    float x = 0.0f;
    float y = 0.0f;          // height above the lane ground (non-zero while jumping)
    int   hp = 1, maxHp = 1;
    float halfWidth = 22.0f;
    float height = 90.0f;
    Pose  pose = Pose::Normal;
    bool  engaged = false;
    bool  alive = true;

    // ---- Member 2's own fields ----
    float attackTimer = 0.0f;   // used by Combat (damage cadence)
    float poseTimer = 0.0f;     // time spent in the current jump / minimum duck time left
    float anim = 0.0f;          // seconds alive, drives walking / idle animation
    float swingT = 0.0f;        // melee attack cycle, 0..kMeleePeriod
    float fireT = 0.0f;         // ranged: seconds until the next shot
    float recoil = 0.0f;        // ranged: crossbow kick, counts down from 1
    float hitFlash = 0.0f;      // brief extra red tint after taking a hit
    float lastAttackTimer = 0.0f;
};

namespace Players
{
    int  maxHpFor(PlayerType t);
    void spawn(World& w, PlayerType t, int lane);
    void update(World& w, float dt);
    void draw(Renderer2D& r, const World& w, const Player& p);
    void drawProjectile(Renderer2D& r, const Projectile& pr);

    // Always use this to hurt a player. Handles sound + death, and redirects the
    // damage to the Defense player in front when the target is protected.
    void damage(World& w, Player& p, int amount);

    // True while a living Defense player in the same lane is to the RIGHT of p.
    bool isProtected(const World& w, const Player& p);

    // Draws one model without needing a World (used by the cards and the cutscene).
    // (x, groundY) = point between the feet. The model faces right.
    //   swing   : -1 = no attack animation, 0..1 = progress of one attack
    //   damage  : 0..1 red tint (use 1 - hp/maxHp)
    //   walking : false = idle breathing pose
    void drawModel(Renderer2D& r, PlayerType t, float x, float groundY, float anim,
                   Pose pose = Pose::Normal, float damage = 0.0f,
                   float swing = -1.0f, bool walking = true);
}

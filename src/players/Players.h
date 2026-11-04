#pragma once
// ============================================================================
//  Players  -  the 3 player types.            Owner: MEMBER 2
//
//  The current .cpp is a PLACEHOLDER (plain boxes, simple walking) so the
//  game runs end-to-end. Replace it with the real models, animations,
//  jump/duck, attacks, and the defense-protection rule.
//
//  Fields marked REQUIRED are read by other members' code - keep their names.
//  Add any extra fields you need below them.
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
    float x = 0.0f;            // centre x (virtual px)
    float y = 0.0f;            // height of the feet above the lane ground (jump offset)
    int   hp = 1, maxHp = 1;
    float halfWidth = 22.0f;   // collision half-width
    float height = 90.0f;      // model height; health bar is drawn just above this
    Pose  pose = Pose::Normal; // Combat checks this to dodge High/Low attacks
    bool  engaged = false;     // currently fighting (stopped)
    bool  alive = true;        // set false to remove (Core erases dead entries)

    // ---- Member 2's own fields ----
    float attackTimer = 0.0f;
    float poseTimer = 0.0f;
    float anim = 0.0f;
};

namespace Players
{
    int  maxHpFor(PlayerType t);
    void spawn(World& w, PlayerType t, int lane);
    void update(World& w, float dt);              // movement, jump/duck input, attacks
    void draw(Renderer2D& r, const World& w, const Player& p);
    void drawProjectile(Renderer2D& r, const Projectile& pr);   // projectiles with fromPlayer == true

    // Applies damage (plays sounds, kills the player at 0 hp). Called by Combat.
    void damage(World& w, Player& p, int amount);

    // Defense rule (spec 3.2): true while a Defense player in the same lane is
    // still alive and is in front of (to the right of) this player.
    bool isProtected(const World& w, const Player& p);
}

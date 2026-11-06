#pragma once
// ============================================================================
//  Players  -  the 3 player types.            Owner: MEMBER 2
//
//   Melee   : survivor with a fire axe (swings once per second)
//   Ranged  : ranger with a crossbow  (fires a bolt every 1.7 s)
//   Defense : riot guard with a big shield (never moves, takes hits for the players behind it)
//
//  Fields marked REQUIRED are read by other members' code - keep their names.
// ============================================================================
#include "../Config.h"

class Renderer2D;
struct World;
struct Projectile;

enum class PlayerType { Melee = 0, Ranged = 1, Defense = 2 };
enum class Pose { Normal, Jumping, Ducking };

struct Player
{
    // ---- REQUIRED (used by Core / Enemies / Combat) ----
    PlayerType type = PlayerType::Melee;
    int   lane = 0;
    float x = 0;
    float y = 0;             // height above the lane ground while jumping
    int   hp = 1, maxHp = 1;
    float halfWidth = 22;
    float height = 90;
    Pose  pose = Pose::Normal;
    bool  engaged = false;   // touching a zombie, so it stops walking
    bool  alive = true;
    bool  strikeNow = false; // melee: the axe lands this frame, Combat deals the damage

    // ---- Member 2's own fields ----
    float anim = 0;          // seconds alive, drives the walk and idle animations
    float poseTime = 0;      // seconds since the current jump or duck started
    float attackTime = 0;    // seconds since the last axe swing or crossbow shot
    float hitFlash = 0;      // extra red flash for a moment after being hit
};

namespace Players
{
    void spawn(World& w, PlayerType t, int lane);
    void update(World& w, float dt);
    void draw(Renderer2D& r, const World& w, const Player& p);
    void drawProjectile(Renderer2D& r, const Projectile& pr);

    // Always use this to hurt a player. If a Defense player stands in front
    // (to the right, same lane), the Defense player takes the hit instead.
    void damage(World& w, Player& p, int amount);

    // Draws one model without a World (used by the cards and the cutscene).
    // (x, groundY) = point between the feet. The model faces right.
    //   swing   : -1 = no attack, 0..1 = progress of one attack
    //   damage  : 0..1 red tint
    //   walking : false = idle pose
    void drawModel(Renderer2D& r, PlayerType t, float x, float groundY, float anim,
                   Pose pose = Pose::Normal, float damage = 0,
                   float swing = -1, bool walking = true);
}

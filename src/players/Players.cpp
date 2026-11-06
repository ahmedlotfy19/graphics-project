// ============================================================================
//  Players  -  Member 2
//  Models + animations, spawning, walking, jump / duck, ranged projectile,
//  the Defense protection rule and damage handling for the 3 player types.
//
//  Checklist (spec):
//    [x] 3 distinct models (silhouette + equipment, not just colour)
//    [x] Melee weapon animates in the hand (axe swing with a trail)
//    [x] Ranged: animated crossbow bolt, disappears on hit
//    [x] Jump (W / Up) and duck (S / Down) in SELECTED lanes, defense ignores it
//    [x] Defense never moves; protects players behind it (isProtected + damage redirect)
//    [x] Red damage tint (+ short hit flash)
//    [x] Attacks automatically from placement, even with no enemy in the lane
//  Notes for Member 3 are marked  "// NOTE M3".
// ============================================================================
#include "Players.h"
#include "../game/World.h"
#include "../core/Renderer2D.h"
#include "../core/Input.h"
#include "../core/Audio.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

namespace
{
    using C = Color;
    inline C rgb(int r, int g, int b, int a = 255) { return Colors::rgb(r, g, b, a); }

    const float kPi = 3.14159265f;

    // ---- gameplay tuning ----
    const float kWalkSpeed    = 45.0f;
    const float kMeleePeriod  = 1.0f;    // seconds per axe swing
    const float kFireInterval = 1.7f;    // seconds between crossbow shots
    const float kBoltSpeed    = 460.0f;
    const float kRecoilTime   = 0.45f;
    const float kJumpTime     = 0.70f;
    const float kJumpHeight   = 38.0f;
    const float kDuckMinTime  = 0.45f;
    const float kDefenseX     = 320.0f;  // Defense stands here, so players spawned at the
                                         // left (PLAYER_SPAWN_X) start BEHIND it (protected)

    // ------------------------------------------------------------------------
    //  small drawing helpers (origin = between the feet, y up, facing +x)
    // ------------------------------------------------------------------------

    // A leg hanging from the hip at (hx, hy); positive angle swings the foot forward.
    void leg(Renderer2D& r, float hx, float hy, float ang, float w, float len, C cloth, C boot, float toe)
    {
        r.push();
        r.translate(hx, hy);
        r.rotate(ang);
        r.rect(-w * 0.5f, -len, w, len, cloth);
        r.roundedRect(-w * 0.5f - 1.0f, -len, w + toe, 7.0f, 2.0f, boot);
        r.pop();
    }

    // An arm hanging from the shoulder, with a round hand at the end.
    void arm(Renderer2D& r, float sx, float sy, float ang, float w, float len, C sleeve, C skin)
    {
        r.push();
        r.translate(sx, sy);
        r.rotate(ang);
        r.rect(-w * 0.5f, -len, w, len, sleeve);
        r.circle(0.0f, -len - 1.0f, w * 0.55f, skin, 14);
        r.pop();
    }

    float smooth01(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }
    float lerp(float a, float b, float t) { return a + (b - a) * t; }

    // Axe angle (radians, the arm hangs down at 0 and swings forward/up as it grows).
    //   2.9 = raised overhead, 0.85 = chopping forward and down.
    float axeAngle(float s)
    {
        if (s < 0.18f) { float u = s / 0.18f; return lerp(2.9f, 0.85f, u * u); }   // fast strike
        if (s < 0.30f) return 0.85f;                                                // impact
        if (s < 0.85f) return lerp(0.85f, 2.9f, smooth01((s - 0.30f) / 0.55f));    // raise again
        return 2.9f;
    }

    // ------------------------------------------------------------------------
    //  MELEE: survivor in an orange hoodie, red bandana, backpack, fire axe
    // ------------------------------------------------------------------------
    void drawMelee(Renderer2D& r, float anim, Pose pose, float swing, bool walking)
    {
        const C skin = rgb(236, 192, 152), hoodie = rgb(196, 98, 45), hoodieDk = rgb(148, 68, 30);
        const C pants = rgb(48, 58, 84), pantsDk = rgb(36, 44, 66), boot = rgb(58, 40, 28);
        const C pack = rgb(78, 98, 62), packDk = rgb(56, 72, 44), leather = rgb(84, 56, 36);
        const C red = rgb(196, 42, 42), hair = rgb(62, 42, 30), dark = rgb(30, 26, 26);
        const C wood = rgb(124, 84, 48), steel = rgb(176, 186, 198), steelLt = rgb(232, 240, 248);

        float s = walking ? std::sin(anim * 8.0f) : 0.0f;
        float bob = walking ? std::fabs(std::cos(anim * 8.0f)) * 1.8f : std::sin(anim * 3.0f) * 0.8f;
        bool air = pose == Pose::Jumping;

        leg(r, -4, 34, air ? -0.55f : -0.55f * s, 10, 34, pantsDk, boot, 5);
        leg(r,  4, 34, air ?  0.85f :  0.55f * s, 10, 34, pants,   boot, 5);

        r.push();
        r.translate(0.0f, bob);

        // backpack (behind the body)
        r.roundedRect(-23, 38, 13, 28, 3, pack);
        r.rect(-23, 56, 13, 3, packDk);
        r.roundedRect(-21, 28, 9, 11, 2, leather);              // rolled sleeping bag

        // off-hand arm
        arm(r, -3, 63, air ? 2.4f : (-0.5f * s - 0.2f), 7, 21, hoodieDk, skin);

        // torso: hoodie, belt, pocket, hood bunched at the neck
        r.roundedRect(-12, 34, 24, 35, 6, hoodie);
        r.rect(-12, 34, 24, 4, leather);
        r.rect(-8, 41, 16, 8, hoodieDk);
        r.rect(2, 50, 1.6f, 17, dark);
        r.circle(-7, 68, 7, hoodieDk, 16);

        // head: skin, hair, red bandana with a knot at the back, eye, mouth
        r.circle(2, 79, 10, skin, 20);
        r.pie(2, 81, 11, 0.5f, 3.0f, hair, 14);
        r.rect(-8, 80.5f, 21, 4, red);
        r.polygon({ { -8, 82 }, { -17, 89 }, { -15, 79 } }, red);
        r.circle(7.5f, 79.5f, 1.7f, dark, 8);
        r.rect(5, 73.5f, 5, 1.4f, dark);

        // weapon arm + fire axe: one rigid piece rotating around the shoulder
        float theta;
        if (air)            theta = 2.7f;
        else if (swing >= 0.0f) theta = axeAngle(swing);
        else                theta = 2.25f + (walking ? 0.12f * s : 0.05f * std::sin(anim * 3.0f));

        if (swing >= 0.0f && swing < 0.32f)                      // motion trail of the swing
        {
            float phi = theta - kPi * 0.5f;                      // direction of the handle
            r.arc(3, 63, 56, 9, phi, std::min(phi + 1.1f, 2.9f - kPi * 0.5f), rgb(255, 255, 255, 80), 14);
        }

        r.push();
        r.translate(3, 63);
        r.rotate(theta);
        r.rect(-3.5f, -22, 7, 22, hoodie);                       // sleeve
        r.circle(0, -23, 4, skin, 12);                           // hand
        r.rect(-2, -66, 4, 52, wood);                            // handle
        r.rect(-2.6f, -27, 5.2f, 3, leather);                    // grip wrap
        r.rect(-2.6f, -33, 5.2f, 3, leather);
        r.polygon({ { 0, -50 }, { -17, -45 }, { -17, -69 }, { 0, -65 } }, steel);   // axe blade
        r.rect(-17.5f, -69, 2.6f, 24, steelLt);                                    // sharp edge
        r.polygon({ { 0, -53 }, { 7, -57 }, { 0, -61 } }, steel);                  // back spike
        r.pop();

        r.pop();
    }

    // ------------------------------------------------------------------------
    //  RANGED: ranger in a long green coat, wide-brim hat, quiver, crossbow
    // ------------------------------------------------------------------------
    void drawRanged(Renderer2D& r, float anim, Pose pose, float swing, bool walking)
    {
        const C skin = rgb(238, 196, 158), coat = rgb(54, 96, 72), coatDk = rgb(38, 70, 52);
        const C pants = rgb(60, 56, 70), boot = rgb(70, 48, 32), leather = rgb(112, 70, 40);
        const C hat = rgb(116, 84, 52), hatDk = rgb(86, 60, 36), band = rgb(176, 48, 40);
        const C scarf = rgb(226, 180, 60), dark = rgb(28, 24, 24), cream = rgb(238, 226, 190);
        const C steel = rgb(150, 158, 170), wood = rgb(136, 94, 52), woodDk = rgb(98, 66, 36);

        float s = walking ? std::sin(anim * 8.0f) : 0.0f;
        float bob = walking ? std::fabs(std::cos(anim * 8.0f)) * 1.6f : std::sin(anim * 3.0f) * 0.7f;
        bool air = pose == Pose::Jumping;

        leg(r, -4, 30, air ? -0.5f : -0.5f * s, 9, 30, pants, boot, 5);
        leg(r,  4, 30, air ?  0.8f :  0.5f * s, 9, 30, pants, boot, 5);

        r.push();
        r.translate(0.0f, bob);

        // quiver on the back with arrows
        r.push();
        r.translate(-13, 58);
        r.rotate(0.4f);
        r.roundedRect(-4.5f, -10, 9, 28, 2, leather);
        r.rect(-2.5f, 18, 1.6f, 7, cream);
        r.rect(0.8f, 18, 1.6f, 9, cream);
        r.rect(-3.0f, 25, 3.2f, 3, band);                       // red fletching
        r.rect(0.2f, 27, 3.2f, 3, band);
        r.pop();

        // long coat: wide at the hem so the silhouette is clearly different from melee
        r.polygon({ { -12, 69 }, { 12, 69 }, { 17, 24 }, { -17, 24 } }, coat);
        r.polygon({ { -1, 69 }, { 3, 69 }, { 4, 24 }, { 0, 24 } }, coatDk);   // coat opening
        r.rect(-14.5f, 46, 29, 4, leather);                                    // belt
        r.rect(-2, 45, 4.5f, 6, rgb(220, 190, 80));                            // buckle
        r.line({ -10, 66 }, { 9, 41 }, 3, leather);                            // quiver strap
        r.polygon({ { -10, 69 }, { 10, 69 }, { 8, 76 }, { -8, 76 } }, scarf);  // scarf

        // head + wide-brim hat
        r.circle(2, 81, 9.5f, skin, 20);
        r.circle(7.5f, 81, 1.6f, dark, 8);
        r.rect(5.5f, 75.5f, 4.5f, 1.3f, dark);
        r.ellipse(2, 88, 19, 4.5f, hat, 20);
        r.roundedRect(-7, 87, 18, 11, 3, hat);
        r.rect(-7, 88.5f, 18, 3, band);
        r.ellipse(2, 85.8f, 19, 1.4f, hatDk, 20);

        // off-hand arm reaching to the crossbow stock
        r.line({ -2, 62 }, { 17, 56 }, 6, coatDk);
        r.circle(18, 56, 3.4f, skin, 10);

        // crossbow, kicks back when it fires
        float kick = (swing >= 0.0f && swing < 0.3f) ? (1.0f - swing / 0.3f) * 5.0f : 0.0f;
        bool loaded = (swing < 0.0f || swing > 0.75f);
        r.push();
        r.translate(-kick, 0.0f);
        r.roundedRect(8, 55, 36, 6, 2, wood);                  // stock
        r.rect(3, 52, 8, 11, woodDk);                          // butt
        r.rect(14, 60, 28, 2.5f, steel);                       // rail
        r.line({ 42, 58 }, { 38, 75 }, 4, steel);              // limbs (prod)
        r.line({ 42, 58 }, { 38, 41 }, 4, steel);
        if (loaded)
        {
            r.line({ 38, 75 }, { 27, 58 }, 1.4f, cream);       // drawn string
            r.line({ 27, 58 }, { 38, 41 }, 1.4f, cream);
            r.rect(26, 61.5f, 22, 2.2f, cream);                // bolt on the rail
            r.polygon({ { 47, 60.2f }, { 55, 62.6f }, { 47, 65 } }, steel);
        }
        else
        {
            r.line({ 38, 75 }, { 38, 41 }, 1.4f, cream);       // slack string after the shot
        }
        r.pop();

        // front arm holding the crossbow
        r.line({ 4, 62 }, { 26 - kick, 58 }, 6.5f, coat);
        r.circle(27 - kick, 58, 3.6f, skin, 10);

        r.pop();
    }

    // ------------------------------------------------------------------------
    //  DEFENSE: bulky riot guard with a helmet and a huge shield
    // ------------------------------------------------------------------------
    void drawDefense(Renderer2D& r, float anim, float swing)
    {
        const C armor = rgb(72, 84, 108), armorLt = rgb(100, 116, 144), armorDk = rgb(46, 54, 72);
        const C pad = rgb(124, 140, 170), boot = rgb(36, 36, 44), skin = rgb(226, 184, 148);
        const C visor = rgb(130, 200, 226), dark = rgb(26, 26, 32);
        const C frame = rgb(150, 160, 176), glass = rgb(150, 205, 232, 215), plate = rgb(110, 124, 142);
        const C yellow = rgb(240, 200, 40), black = rgb(34, 34, 38);

        float breathe = std::sin(anim * 2.4f) * 0.8f;
        float bash = (swing >= 0.0f) ? std::sin(swing * kPi) * 6.0f : 0.0f;

        // wide, planted legs
        leg(r, -9, 30, 0.0f, 13, 30, armorDk, boot, 6);
        leg(r,  9, 30, 0.0f, 13, 30, armor,   boot, 6);

        r.push();
        r.translate(0.0f, breathe);

        // torso with chest plate and belt
        r.roundedRect(-18, 30, 36, 41, 6, armor);
        r.roundedRect(-13, 38, 26, 27, 4, armorLt);
        r.rect(-5, 38, 10, 27, armor);
        r.rect(-18, 30, 36, 5, dark);
        r.rect(-3, 29.5f, 7, 6, rgb(220, 190, 80));

        // shoulder pads
        r.pie(-17, 68, 11, 0.0f, kPi, pad, 14);
        r.pie(17, 68, 11, 0.0f, kPi, pad, 14);
        r.rect(-28, 66, 22, 3, armorDk);
        r.rect(6, 66, 22, 3, armorDk);

        // head and helmet with a visor
        r.circle(1, 79, 10, skin, 18);
        r.pie(1, 79, 13, 0.0f, kPi, armorDk, 16);
        r.rect(-12, 77, 26, 4, armorDk);
        r.rect(3, 73, 12, 6, visor);
        r.rect(3, 72, 12, 1.4f, dark);
        r.rect(-1, 66, 10, 6, armorDk);

        // arm holding the shield
        r.line({ 14, 60 }, { 26 + bash, 50 }, 9, armor);
        r.circle(27 + bash, 50, 5.5f, dark, 12);

        // the shield: frame, glass window, plate, rivets, hazard stripes
        r.push();
        r.translate(bash, 0.0f);
        r.roundedRect(21, 4, 19, 88, 6, frame);
        r.roundedRect(24, 52, 13, 35, 4, glass);
        r.roundedRect(24, 14, 13, 34, 3, plate);
        for (int i = 0; i < 4; ++i)
            r.rect(24, 6 + i * 2.6f, 13, 1.3f, (i % 2 == 0) ? yellow : black);
        r.circle(30.5f, 91, 1.6f, dark, 8);
        r.circle(30.5f, 8.2f, 1.6f, dark, 8);
        r.circle(24.2f, 49.5f, 1.4f, dark, 8);
        r.circle(36.8f, 49.5f, 1.4f, dark, 8);
        r.pop();

        r.pop();
    }

    // ------------------------------------------------------------------------
    //  gameplay helpers
    // ------------------------------------------------------------------------
    const Player* defenderInFront(const World& w, const Player& p)
    {
        const Player* best = nullptr;
        for (const Player& q : w.players)
        {
            if (&q == &p || !q.alive || q.type != PlayerType::Defense || q.lane != p.lane) continue;
            if (q.x > p.x && (!best || q.x < best->x)) best = &q;
        }
        return best;
    }
}

namespace Players
{
    int maxHpFor(PlayerType t)
    {
        switch (t)
        {
        case PlayerType::Defense: return DEFENSE_PLAYER_HP;
        case PlayerType::Ranged:  return RANGED_PLAYER_HP;
        default:                  return MELEE_PLAYER_HP;
        }
    }

    void spawn(World& w, PlayerType t, int lane)
    {
        Player p;
        p.type = t;
        p.lane = lane;
        p.x = (t == PlayerType::Defense) ? kDefenseX : PLAYER_SPAWN_X;
        p.hp = p.maxHp = maxHpFor(t);
        switch (t)
        {
        case PlayerType::Melee:   p.halfWidth = 20.0f; p.height = 90.0f; break;
        case PlayerType::Ranged:  p.halfWidth = 22.0f; p.height = 98.0f; break;
        case PlayerType::Defense: p.halfWidth = 30.0f; p.height = 92.0f; break;
        default: break;
        }
        p.swingT = 0.9f;
        p.fireT = 0.6f;                       // first shot shortly after being placed
        w.players.push_back(p);
        Audio::play(Sfx::Place);
    }

    void update(World& w, float dt)
    {
        bool jumpKey = Input::keyPressed(GLFW_KEY_W) || Input::keyPressed(GLFW_KEY_UP);
        bool duckKey = Input::keyPressed(GLFW_KEY_S) || Input::keyPressed(GLFW_KEY_DOWN);
        bool duckHeld = Input::keyDown(GLFW_KEY_S) || Input::keyDown(GLFW_KEY_DOWN);
        bool jumpedThisFrame = false;

        for (Player& p : w.players)
        {
            if (!p.alive) continue;
            p.anim += dt;
            p.hitFlash = std::max(0.0f, p.hitFlash - dt);
            bool defense = p.type == PlayerType::Defense;

            // touching a zombie in the same lane -> stop (Combat does the damage)
            p.engaged = false;
            for (const Enemy& e : w.enemies)
                if (e.alive && e.lane == p.lane && std::fabs(e.x - p.x) < p.halfWidth + e.halfWidth)
                    p.engaged = true;

            // auto-advance left -> right; Defense never moves
            if (!defense && !p.engaged)
                p.x += kWalkSpeed * dt;

            if (p.x - p.halfWidth > FIELD_RIGHT)
            {
                p.alive = false;
                continue;
            }

            // ---- jump / duck: only players in SELECTED lanes, never Defense ----
            bool selected = w.lanes.selected[p.lane];
            if (!defense)
            {
                if (p.pose == Pose::Normal && selected)
                {
                    if (jumpKey)      { p.pose = Pose::Jumping; p.poseTimer = 0.0f; jumpedThisFrame = true; }
                    else if (duckKey) { p.pose = Pose::Ducking; p.poseTimer = kDuckMinTime; }
                }
                if (p.pose == Pose::Jumping)
                {
                    p.poseTimer += dt;
                    float u = p.poseTimer / kJumpTime;
                    if (u >= 1.0f) { p.pose = Pose::Normal; p.y = 0.0f; p.poseTimer = 0.0f; }
                    else           p.y = 4.0f * kJumpHeight * u * (1.0f - u);
                }
                else if (p.pose == Pose::Ducking)
                {
                    p.poseTimer -= dt;
                    if (p.poseTimer <= 0.0f && !(duckHeld && selected)) { p.pose = Pose::Normal; p.poseTimer = 0.0f; }
                }
            }

            // ---- melee: the axe swings continuously (attacks even with no enemy) ----
            if (p.type == PlayerType::Melee)
            {
                // NOTE M3: when Combat applies a hit it resets p.attackTimer upwards; we notice that
                // and restart the swing so the animation lines up with the real hit.
                bool restarted = false;
                if (p.attackTimer > p.lastAttackTimer + 0.5f) { p.swingT = 0.0f; restarted = true; }
                p.lastAttackTimer = p.attackTimer;

                p.swingT += dt;
                if (p.swingT >= kMeleePeriod) { p.swingT -= kMeleePeriod; restarted = true; }
                if (restarted && p.engaged) Audio::play(Sfx::PlayerAttack);
            }

            // ---- ranged: fire a bolt every kFireInterval, even with no enemy in the lane ----
            if (p.type == PlayerType::Ranged)
            {
                p.recoil = std::max(0.0f, p.recoil - dt / kRecoilTime);
                p.fireT -= dt;
                if (p.fireT <= 0.0f)
                {
                    p.fireT = kFireInterval;
                    p.recoil = 1.0f;
                    Projectile b;
                    b.fromPlayer = true;
                    b.lane = p.lane;
                    b.x = p.x + 46.0f;
                    b.y = laneGroundY(p.lane) + p.y + (p.pose == Pose::Ducking ? 40.0f : 60.0f);
                    b.vx = kBoltSpeed;
                    b.vy = 0.0f;
                    b.damage = PLAYER_DAMAGE;
                    b.height = AttackHeight::Mid;
                    w.projectiles.push_back(b);
                    Audio::play(Sfx::PlayerAttack);
                }
            }
        }

        if (jumpedThisFrame) Audio::play(Sfx::Jump);

        // ---- our bolts hit zombies (1 dmg) and disappear ----
        // NOTE M3: Combat::update still moves ALL projectiles. This is only the hit test for
        // player bolts; if you add your own, nothing is hit twice because we set alive = false.
        for (Projectile& pr : w.projectiles)
        {
            if (!pr.alive || !pr.fromPlayer) continue;
            for (Enemy& e : w.enemies)
            {
                if (!e.alive || e.lane != pr.lane) continue;
                float ground = laneGroundY(e.lane);
                if (std::fabs(pr.x - e.x) < e.halfWidth + 8.0f && pr.y >= ground - 4.0f && pr.y <= ground + e.height + 4.0f)
                {
                    Enemies::damage(w, e, pr.damage);
                    pr.alive = false;
                    break;
                }
            }
        }
    }

    void drawModel(Renderer2D& r, PlayerType t, float x, float groundY, float anim,
                   Pose pose, float damage, float swing, bool walking)
    {
        r.push();
        r.translate(x, groundY);
        if (pose == Pose::Ducking && t != PlayerType::Defense)
            r.scale(1.08f, 0.68f);                          // crouch
        r.setDamageTint(damage);                            // set AFTER push(), BEFORE the shapes

        switch (t)
        {
        case PlayerType::Melee:   drawMelee(r, anim, pose, swing, walking);   break;
        case PlayerType::Ranged:  drawRanged(r, anim, pose, swing, walking);  break;
        case PlayerType::Defense: drawDefense(r, anim, swing);                break;
        default: break;
        }
        r.pop();
    }

    void draw(Renderer2D& r, const World&, const Player& p)
    {
        float ground = laneGroundY(p.lane);

        // ground shadow shrinks while jumping
        float k = std::max(0.4f, 1.0f - p.y / 90.0f);
        float sw = (p.type == PlayerType::Defense ? 32.0f : 22.0f) * k;
        r.ellipse(p.x, ground - 1.0f, sw, 5.0f * k, Color(0, 0, 0, 0.30f), 20);

        float damage = 1.0f - (float)p.hp / (float)p.maxHp;
        if (p.hitFlash > 0.0f) damage = std::max(damage, 0.45f);

        float swing = -1.0f;
        if (p.type == PlayerType::Melee)       swing = p.swingT / kMeleePeriod;
        else if (p.type == PlayerType::Ranged) swing = (p.recoil > 0.0f) ? 1.0f - p.recoil : -1.0f;

        bool walking = !p.engaged && p.type != PlayerType::Defense;
        drawModel(r, p.type, p.x, ground + p.y, p.anim, p.pose, damage, swing, walking);
    }

    void drawProjectile(Renderer2D& r, const Projectile& pr)
    {
        // crossbow bolt: glowing trail, shaft, steel head, spinning fletching
        float flutter = 3.5f + 2.0f * std::sin(pr.age * 40.0f);
        float pulse = 0.5f + 0.5f * std::sin(pr.age * 18.0f);
        r.push();
        r.translate(pr.x, pr.y);
        r.rotate(std::atan2(pr.vy, pr.vx));
        r.rect(-44, -1.0f, 34, 2.0f, rgb(255, 240, 170, 30));
        r.rect(-30, -1.4f, 22, 2.8f, rgb(255, 240, 170, 60));
        r.circle(11, 0, 6 + pulse * 2.0f, rgb(255, 220, 120, 55), 14);
        r.rect(-14, -1.5f, 24, 3, rgb(238, 226, 190));
        r.polygon({ { 9, -4.5f }, { 19, 0 }, { 9, 4.5f } }, rgb(176, 186, 198));
        r.polygon({ { -14, 0 }, { -21, -flutter }, { -17, 0 } }, rgb(196, 60, 52));
        r.polygon({ { -14, 0 }, { -21, flutter }, { -17, 0 } }, rgb(196, 60, 52));
        r.pop();
    }

    void damage(World& w, Player& p, int amount)
    {
        if (!p.alive) return;

        // a protected player is shielded: the Defense player in front takes the hit instead
        if (const Player* d = defenderInFront(w, p))
        {
            damage(w, const_cast<Player&>(*d), amount);
            return;
        }

        p.hp -= amount;
        p.hitFlash = 0.18f;
        Audio::play(Sfx::Hit);
        if (p.hp <= 0)
        {
            p.hp = 0;
            p.alive = false;
            Audio::play(Sfx::PlayerDeath);
        }
    }

    bool isProtected(const World& w, const Player& p)
    {
        return defenderInFront(w, p) != nullptr;
    }
}

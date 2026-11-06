// ============================================================================
//  Players  -  Member 2
//  Models + animations, spawning, walking, jump / duck, shooting bolts
//  and the Defense protection rule for the 3 player types.
//
//  Checklist (spec):
//    [x] 3 distinct models (silhouette + equipment, not just colour)
//    [x] Melee weapon animates in the hand (axe swing with a trail)
//    [x] Ranged: animated crossbow bolt, disappears on hit (hit test in Combat.cpp)
//    [x] Jump (W / Up) and duck (S / Down) in SELECTED lanes, defense ignores it
//    [x] Defense never moves; protects players behind it (damage redirect)
//    [x] Red damage tint (+ short hit flash)
//    [x] Attacks automatically from placement, even with no enemy in the lane
// ============================================================================
#include "Players.h"
#include "../game/World.h"
#include "../core/Renderer2D.h"
#include "../core/Input.h"
#include "../core/Audio.h"

#include <cmath>

using Colors::rgb;

static const float PI = 3.14159265f;

static const float WALK_SPEED    = 45;
static const float SWING_TIME    = 1.0f;
static const float FIRE_TIME     = 1.7f;
static const float RECOIL_TIME   = 0.45f;
static const float BOLT_SPEED    = 460;
static const float JUMP_TIME     = 0.7f;
static const float JUMP_HEIGHT   = 38;
static const float MIN_DUCK_TIME = 0.45f;
static const float DEFENSE_X     = 320;

static void leg(Renderer2D& r, float hipX, float hipY, float angle, float width, float length,
                Color cloth, Color boot, float toe)
{
    r.push();
    r.translate(hipX, hipY);
    r.rotate(angle);
    r.rect(-width / 2, -length, width, length, cloth);
    r.roundedRect(-width / 2 - 1, -length, width + toe, 7, 2, boot);
    r.pop();
}

static void arm(Renderer2D& r, float shoulderX, float shoulderY, float angle, float width, float length,
                Color sleeve, Color skin)
{
    r.push();
    r.translate(shoulderX, shoulderY);
    r.rotate(angle);
    r.rect(-width / 2, -length, width, length, sleeve);
    r.circle(0, -length - 1, width * 0.55f, skin, 14);
    r.pop();
}

static float axeAngle(float swing)
{
    float raised = 2.9f;
    float chopped = 0.85f;

    if (swing < 0.18f) return raised - (raised - chopped) * (swing / 0.18f);
    if (swing < 0.30f) return chopped;
    if (swing < 0.85f) return chopped + (raised - chopped) * ((swing - 0.30f) / 0.55f);
    return raised;
}

static void drawMelee(Renderer2D& r, float anim, Pose pose, float swing, bool walking)
{
    Color skin = rgb(236, 192, 152), hoodie = rgb(196, 98, 45), hoodieDark = rgb(148, 68, 30);
    Color pants = rgb(48, 58, 84), pantsDark = rgb(36, 44, 66), boot = rgb(58, 40, 28);
    Color pack = rgb(78, 98, 62), packDark = rgb(56, 72, 44), leather = rgb(84, 56, 36);
    Color red = rgb(196, 42, 42), hair = rgb(62, 42, 30), dark = rgb(30, 26, 26);
    Color wood = rgb(124, 84, 48), steel = rgb(176, 186, 198), steelLight = rgb(232, 240, 248);

    float step = walking ? sin(anim * 8) : 0;
    float bob = walking ? fabs(cos(anim * 8)) * 1.8f : sin(anim * 3) * 0.8f;
    bool inAir = pose == Pose::Jumping;

    leg(r, -4, 34, inAir ? -0.55f : -0.55f * step, 10, 34, pantsDark, boot, 5);
    leg(r,  4, 34, inAir ?  0.85f :  0.55f * step, 10, 34, pants, boot, 5);

    r.push();
    r.translate(0, bob);

    r.roundedRect(-23, 38, 13, 28, 3, pack);
    r.rect(-23, 56, 13, 3, packDark);
    r.roundedRect(-21, 28, 9, 11, 2, leather);

    arm(r, -3, 63, inAir ? 2.4f : -0.5f * step - 0.2f, 7, 21, hoodieDark, skin);

    r.roundedRect(-12, 34, 24, 35, 6, hoodie);
    r.rect(-12, 34, 24, 4, leather);
    r.rect(-8, 41, 16, 8, hoodieDark);
    r.rect(2, 50, 1.6f, 17, dark);
    r.circle(-7, 68, 7, hoodieDark, 16);

    r.circle(2, 79, 10, skin, 20);
    r.pie(2, 81, 11, 0.5f, 3.0f, hair, 14);
    r.rect(-8, 80.5f, 21, 4, red);
    r.polygon({ { -8, 82 }, { -17, 89 }, { -15, 79 } }, red);
    r.circle(7.5f, 79.5f, 1.7f, dark, 8);
    r.rect(5, 73.5f, 5, 1.4f, dark);

    float angle = 2.25f + (walking ? 0.12f * step : 0.05f * sin(anim * 3));
    if (swing >= 0) angle = axeAngle(swing);
    if (inAir)      angle = 2.7f;

    if (swing >= 0 && swing < 0.32f)
    {
        float start = angle - PI / 2;
        float end = fmin(start + 1.1f, 2.9f - PI / 2);
        r.arc(3, 63, 56, 9, start, end, rgb(255, 255, 255, 80), 14);
    }

    r.push();
    r.translate(3, 63);
    r.rotate(angle);
    r.rect(-3.5f, -22, 7, 22, hoodie);
    r.circle(0, -23, 4, skin, 12);
    r.rect(-2, -66, 4, 52, wood);
    r.rect(-2.6f, -27, 5.2f, 3, leather);
    r.rect(-2.6f, -33, 5.2f, 3, leather);
    r.polygon({ { 0, -50 }, { -17, -45 }, { -17, -69 }, { 0, -65 } }, steel);
    r.rect(-17.5f, -69, 2.6f, 24, steelLight);
    r.polygon({ { 0, -53 }, { 7, -57 }, { 0, -61 } }, steel);
    r.pop();

    r.pop();
}

static void drawRanged(Renderer2D& r, float anim, Pose pose, float swing, bool walking)
{
    Color skin = rgb(238, 196, 158), coat = rgb(54, 96, 72), coatDark = rgb(38, 70, 52);
    Color pants = rgb(60, 56, 70), boot = rgb(70, 48, 32), leather = rgb(112, 70, 40);
    Color hat = rgb(116, 84, 52), hatDark = rgb(86, 60, 36), band = rgb(176, 48, 40);
    Color scarf = rgb(226, 180, 60), dark = rgb(28, 24, 24), cream = rgb(238, 226, 190);
    Color steel = rgb(150, 158, 170), wood = rgb(136, 94, 52), woodDark = rgb(98, 66, 36);

    float step = walking ? sin(anim * 8) : 0;
    float bob = walking ? fabs(cos(anim * 8)) * 1.6f : sin(anim * 3) * 0.7f;
    bool inAir = pose == Pose::Jumping;

    leg(r, -4, 30, inAir ? -0.5f : -0.5f * step, 9, 30, pants, boot, 5);
    leg(r,  4, 30, inAir ?  0.8f :  0.5f * step, 9, 30, pants, boot, 5);

    r.push();
    r.translate(0, bob);

    r.push();
    r.translate(-13, 58);
    r.rotate(0.4f);
    r.roundedRect(-4.5f, -10, 9, 28, 2, leather);
    r.rect(-2.5f, 18, 1.6f, 7, cream);
    r.rect(0.8f, 18, 1.6f, 9, cream);
    r.rect(-3.0f, 25, 3.2f, 3, band);
    r.rect(0.2f, 27, 3.2f, 3, band);
    r.pop();

    r.polygon({ { -12, 69 }, { 12, 69 }, { 17, 24 }, { -17, 24 } }, coat);
    r.polygon({ { -1, 69 }, { 3, 69 }, { 4, 24 }, { 0, 24 } }, coatDark);
    r.rect(-14.5f, 46, 29, 4, leather);
    r.rect(-2, 45, 4.5f, 6, rgb(220, 190, 80));
    r.line({ -10, 66 }, { 9, 41 }, 3, leather);
    r.polygon({ { -10, 69 }, { 10, 69 }, { 8, 76 }, { -8, 76 } }, scarf);

    r.circle(2, 81, 9.5f, skin, 20);
    r.circle(7.5f, 81, 1.6f, dark, 8);
    r.rect(5.5f, 75.5f, 4.5f, 1.3f, dark);
    r.ellipse(2, 88, 19, 4.5f, hat, 20);
    r.roundedRect(-7, 87, 18, 11, 3, hat);
    r.rect(-7, 88.5f, 18, 3, band);
    r.ellipse(2, 85.8f, 19, 1.4f, hatDark, 20);

    r.line({ -2, 62 }, { 17, 56 }, 6, coatDark);
    r.circle(18, 56, 3.4f, skin, 10);

    float kick = 0;
    if (swing >= 0 && swing < 0.3f) kick = (1 - swing / 0.3f) * 5;
    bool loaded = swing < 0 || swing > 0.75f;

    r.push();
    r.translate(-kick, 0);
    r.roundedRect(8, 55, 36, 6, 2, wood);
    r.rect(3, 52, 8, 11, woodDark);
    r.rect(14, 60, 28, 2.5f, steel);
    r.line({ 42, 58 }, { 38, 75 }, 4, steel);
    r.line({ 42, 58 }, { 38, 41 }, 4, steel);
    if (loaded)
    {
        r.line({ 38, 75 }, { 27, 58 }, 1.4f, cream);
        r.line({ 27, 58 }, { 38, 41 }, 1.4f, cream);
        r.rect(26, 61.5f, 22, 2.2f, cream);
        r.polygon({ { 47, 60.2f }, { 55, 62.6f }, { 47, 65 } }, steel);
    }
    else
    {
        r.line({ 38, 75 }, { 38, 41 }, 1.4f, cream);
    }
    r.pop();

    r.line({ 4, 62 }, { 26 - kick, 58 }, 6.5f, coat);
    r.circle(27 - kick, 58, 3.6f, skin, 10);

    r.pop();
}

static void drawDefense(Renderer2D& r, float anim, float swing)
{
    Color armor = rgb(72, 84, 108), armorLight = rgb(100, 116, 144), armorDark = rgb(46, 54, 72);
    Color pad = rgb(124, 140, 170), boot = rgb(36, 36, 44), skin = rgb(226, 184, 148);
    Color visor = rgb(130, 200, 226), dark = rgb(26, 26, 32);
    Color frame = rgb(150, 160, 176), glass = rgb(150, 205, 232, 215), plate = rgb(110, 124, 142);
    Color yellow = rgb(240, 200, 40), black = rgb(34, 34, 38);

    float breathe = sin(anim * 2.4f) * 0.8f;
    float bash = swing >= 0 ? sin(swing * PI) * 6 : 0;

    leg(r, -9, 30, 0, 13, 30, armorDark, boot, 6);
    leg(r,  9, 30, 0, 13, 30, armor, boot, 6);

    r.push();
    r.translate(0, breathe);

    r.roundedRect(-18, 30, 36, 41, 6, armor);
    r.roundedRect(-13, 38, 26, 27, 4, armorLight);
    r.rect(-5, 38, 10, 27, armor);
    r.rect(-18, 30, 36, 5, dark);
    r.rect(-3, 29.5f, 7, 6, rgb(220, 190, 80));

    r.pie(-17, 68, 11, 0, PI, pad, 14);
    r.pie(17, 68, 11, 0, PI, pad, 14);
    r.rect(-28, 66, 22, 3, armorDark);
    r.rect(6, 66, 22, 3, armorDark);

    r.circle(1, 79, 10, skin, 18);
    r.pie(1, 79, 13, 0, PI, armorDark, 16);
    r.rect(-12, 77, 26, 4, armorDark);
    r.rect(3, 73, 12, 6, visor);
    r.rect(3, 72, 12, 1.4f, dark);
    r.rect(-1, 66, 10, 6, armorDark);

    r.line({ 14, 60 }, { 26 + bash, 50 }, 9, armor);
    r.circle(27 + bash, 50, 5.5f, dark, 12);

    r.push();
    r.translate(bash, 0);
    r.roundedRect(21, 4, 19, 88, 6, frame);
    r.roundedRect(24, 52, 13, 35, 4, glass);
    r.roundedRect(24, 14, 13, 34, 3, plate);
    for (int i = 0; i < 4; i++)
        r.rect(24, 6 + i * 2.6f, 13, 1.3f, i % 2 == 0 ? yellow : black);
    r.circle(30.5f, 91, 1.6f, dark, 8);
    r.circle(30.5f, 8.2f, 1.6f, dark, 8);
    r.circle(24.2f, 49.5f, 1.4f, dark, 8);
    r.circle(36.8f, 49.5f, 1.4f, dark, 8);
    r.pop();

    r.pop();
}

static bool touchingEnemy(const World& w, const Player& p)
{
    for (const Enemy& e : w.enemies)
        if (e.alive && e.lane == p.lane && fabs(e.x - p.x) < p.halfWidth + e.halfWidth)
            return true;
    return false;
}

static Player* defenderInFront(World& w, const Player& p)
{
    Player* nearest = nullptr;
    for (Player& d : w.players)
    {
        if (&d == &p || !d.alive || d.type != PlayerType::Defense) continue;
        if (d.lane != p.lane || d.x <= p.x) continue;
        if (!nearest || d.x < nearest->x) nearest = &d;
    }
    return nearest;
}

static void shootBolt(World& w, const Player& p)
{
    Projectile bolt;
    bolt.fromPlayer = true;
    bolt.lane = p.lane;
    bolt.x = p.x + 46;
    bolt.y = laneGroundY(p.lane) + p.y + (p.pose == Pose::Ducking ? 40 : 60);
    bolt.vx = BOLT_SPEED;
    bolt.damage = PLAYER_DAMAGE;
    bolt.height = AttackHeight::Mid;
    w.projectiles.push_back(bolt);
    Audio::play(Sfx::PlayerAttack);
}

namespace Players
{
    void spawn(World& w, PlayerType t, int lane)
    {
        Player p;
        p.type = t;
        p.lane = lane;
        p.x = PLAYER_SPAWN_X;

        if (t == PlayerType::Melee)
        {
            p.hp = MELEE_PLAYER_HP;
            p.halfWidth = 20;
            p.height = 90;
            p.attackTime = 0.9f;
        }
        else if (t == PlayerType::Ranged)
        {
            p.hp = RANGED_PLAYER_HP;
            p.halfWidth = 22;
            p.height = 98;
            p.attackTime = FIRE_TIME - 0.6f;
        }
        else
        {
            p.hp = DEFENSE_PLAYER_HP;
            p.halfWidth = 30;
            p.height = 92;
            p.x = DEFENSE_X;
        }
        p.maxHp = p.hp;

        w.players.push_back(p);
        Audio::play(Sfx::Place);
    }

    void update(World& w, float dt)
    {
        bool jumpPressed = Input::keyPressed(GLFW_KEY_W) || Input::keyPressed(GLFW_KEY_UP);
        bool duckPressed = Input::keyPressed(GLFW_KEY_S) || Input::keyPressed(GLFW_KEY_DOWN);
        bool duckHeld = Input::keyDown(GLFW_KEY_S) || Input::keyDown(GLFW_KEY_DOWN);
        bool someoneJumped = false;

        for (Player& p : w.players)
        {
            if (!p.alive) continue;

            p.anim += dt;
            p.poseTime += dt;
            p.attackTime += dt;
            p.hitFlash -= dt;

            p.engaged = touchingEnemy(w, p);

            bool defense = p.type == PlayerType::Defense;
            if (!defense && !p.engaged)
                p.x += WALK_SPEED * dt;

            if (p.x - p.halfWidth > FIELD_RIGHT)
            {
                p.alive = false;
                continue;
            }

            bool selected = w.lanes.selected[p.lane];
            if (!defense && selected && p.pose == Pose::Normal)
            {
                if (jumpPressed)
                {
                    p.pose = Pose::Jumping;
                    p.poseTime = 0;
                    someoneJumped = true;
                }
                else if (duckPressed)
                {
                    p.pose = Pose::Ducking;
                    p.poseTime = 0;
                }
            }

            if (p.pose == Pose::Jumping)
            {
                p.y = JUMP_HEIGHT * sin(PI * p.poseTime / JUMP_TIME);
                if (p.poseTime >= JUMP_TIME)
                {
                    p.pose = Pose::Normal;
                    p.y = 0;
                }
            }

            if (p.pose == Pose::Ducking && p.poseTime >= MIN_DUCK_TIME && !(duckHeld && selected))
                p.pose = Pose::Normal;

            if (p.type == PlayerType::Melee && p.attackTime >= SWING_TIME)
            {
                p.attackTime = 0;
                p.strikeNow = true;
                if (p.engaged) Audio::play(Sfx::PlayerAttack);
            }

            if (p.type == PlayerType::Ranged && p.attackTime >= FIRE_TIME)
            {
                p.attackTime = 0;
                shootBolt(w, p);
            }
        }

        if (someoneJumped) Audio::play(Sfx::Jump);
    }

    void drawModel(Renderer2D& r, PlayerType t, float x, float groundY, float anim,
                   Pose pose, float damage, float swing, bool walking)
    {
        r.push();
        r.translate(x, groundY);
        if (pose == Pose::Ducking && t != PlayerType::Defense)
            r.scale(1.08f, 0.68f);
        r.setDamageTint(damage);

        if (t == PlayerType::Melee)       drawMelee(r, anim, pose, swing, walking);
        else if (t == PlayerType::Ranged) drawRanged(r, anim, pose, swing, walking);
        else                              drawDefense(r, anim, swing);

        r.pop();
    }

    void draw(Renderer2D& r, const World&, const Player& p)
    {
        float ground = laneGroundY(p.lane);

        float shadow = 1 - p.y / 90;
        float shadowWidth = p.type == PlayerType::Defense ? 32 : 22;
        r.ellipse(p.x, ground - 1, shadowWidth * shadow, 5 * shadow, Color(0, 0, 0, 0.3f), 20);

        float damage = 1.0f - (float)p.hp / p.maxHp;
        if (p.hitFlash > 0 && damage < 0.45f) damage = 0.45f;

        float swing = -1;
        if (p.type == PlayerType::Melee)
            swing = p.attackTime / SWING_TIME;
        if (p.type == PlayerType::Ranged && p.attackTime < RECOIL_TIME)
            swing = p.attackTime / RECOIL_TIME;

        bool walking = !p.engaged && p.type != PlayerType::Defense;
        drawModel(r, p.type, p.x, ground + p.y, p.anim, p.pose, damage, swing, walking);
    }

    void drawProjectile(Renderer2D& r, const Projectile& pr)
    {
        float flutter = 3.5f + 2 * sin(pr.age * 40);
        float pulse = 0.5f + 0.5f * sin(pr.age * 18);

        r.push();
        r.translate(pr.x, pr.y);
        r.rect(-44, -1, 34, 2, rgb(255, 240, 170, 30));
        r.rect(-30, -1.4f, 22, 2.8f, rgb(255, 240, 170, 60));
        r.circle(11, 0, 6 + pulse * 2, rgb(255, 220, 120, 55), 14);
        r.rect(-14, -1.5f, 24, 3, rgb(238, 226, 190));
        r.polygon({ { 9, -4.5f }, { 19, 0 }, { 9, 4.5f } }, rgb(176, 186, 198));
        r.polygon({ { -14, 0 }, { -21, -flutter }, { -17, 0 } }, rgb(196, 60, 52));
        r.polygon({ { -14, 0 }, { -21, flutter }, { -17, 0 } }, rgb(196, 60, 52));
        r.pop();
    }

    void damage(World& w, Player& p, int amount)
    {
        if (!p.alive) return;

        Player* defender = defenderInFront(w, p);
        if (defender)
        {
            damage(w, *defender, amount);
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
}

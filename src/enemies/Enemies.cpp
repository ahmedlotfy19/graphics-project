#include "Enemies.h"
#include "../game/World.h"
#include "../game/Collectibles.h"
#include "../core/Renderer2D.h"
#include "../core/Audio.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    const float kPi = 3.14159265f;

    const float kSpitSpeed   = 270.0f;
    const float kSkullSpeed  = 220.0f;
    const float kBruteHitAt  = 0.17f;

    Color rgb(int r, int g, int b, int a = 255) { return Colors::rgb(r, g, b, a); }

    float speedFor(EnemyType t)
    {
        switch (t)
        {
        case EnemyType::Melee:       return 72.0f;
        case EnemyType::LowerRanged: return 26.0f;
        default:                     return 30.0f;
        }
    }

    float attackDuration(EnemyType t)
    {
        switch (t)
        {
        case EnemyType::Melee:       return 0.55f;
        case EnemyType::LowerRanged: return 0.45f;
        default:                     return 0.40f;
        }
    }

    float randRange(float a, float b) { return a + (b - a) * (std::rand() / (float)RAND_MAX); }

    void leg(Renderer2D& r, float hx, float hy, float a1, float a2, float l1, float l2,
             float t, Color c, Color foot, float footLen)
    {
        r.push();
        r.translate(hx, hy);
        r.rotate(a1);
        r.roundedRect(-t * 0.5f, -l1, t, l1 + 2, t * 0.45f, c);
        r.translate(0, -l1);
        r.rotate(a2);
        r.roundedRect(-t * 0.45f, -l2, t * 0.9f, l2 + 2, t * 0.4f, c);
        r.translate(0, -l2);
        r.rotate(-(a1 + a2));
        r.roundedRect(-t * 0.5f - footLen, -3, t + footLen + 2, 7, 3, foot);
        r.pop();
    }

    void arm(Renderer2D& r, float sx, float sy, float a1, float a2, float l1, float l2,
             float t, Color upper, Color lower, float handR)
    {
        r.push();
        r.translate(sx, sy);
        r.rotate(a1);
        r.roundedRect(-t * 0.5f, -l1, t, l1 + 2, t * 0.45f, upper);
        r.translate(0, -l1);
        r.rotate(a2);
        r.roundedRect(-t * 0.45f, -l2, t * 0.9f, l2 + 2, t * 0.4f, lower);
        r.circle(0, -l2, handR, lower, 12);
        r.line({ 0, -l2 }, { -handR * 0.6f, -l2 - handR * 1.4f }, 1.4f, rgb(220, 210, 170));
        r.line({ 0, -l2 }, { handR * 0.5f, -l2 - handR * 1.5f }, 1.4f, rgb(220, 210, 170));
        r.pop();
    }

    void skull(Renderer2D& r, float x, float y, float rad, float rot)
    {
        Color bone = rgb(234, 228, 206), boneD = rgb(196, 188, 160), hole = rgb(40, 30, 30);
        r.push();
        r.translate(x, y);
        r.rotate(rot);
        r.circle(0, 0, rad, bone, 18);
        r.roundedRect(-rad * 0.55f, -rad * 1.05f, rad * 1.1f, rad * 0.6f, rad * 0.2f, boneD);
        r.circle(-rad * 0.4f, rad * 0.05f, rad * 0.28f, hole, 10);
        r.circle(rad * 0.4f, rad * 0.05f, rad * 0.28f, hole, 10);
        r.triangle({ -rad * 0.12f, -rad * 0.3f }, { rad * 0.12f, -rad * 0.3f }, { 0, -rad * 0.08f }, hole);
        for (int i = 0; i < 3; ++i)
            r.rect(-rad * 0.4f + i * rad * 0.3f, -rad * 0.9f, rad * 0.08f + 0.6f, rad * 0.35f, hole);
        r.line({ rad * 0.2f, rad * 0.9f }, { rad * 0.45f, rad * 0.45f }, 1.0f, boneD);
        r.pop();
    }

    void drawSpitter(Renderer2D& r, float anim, bool walking, float attack, float charge)
    {
        Color skin = rgb(132, 163, 100), skinD = rgb(96, 124, 74);
        Color shirt = rgb(112, 86, 122), shirtD = rgb(80, 58, 90);
        Color pants = rgb(70, 68, 84), pantsD = rgb(52, 50, 64), foot = rgb(58, 46, 40);
        Color hair = rgb(58, 50, 44), acid = rgb(176, 214, 72);

        float s = walking ? std::sin(anim * 5.0f) : 0.0f;
        float bob = walking ? std::fabs(std::cos(anim * 5.0f)) * 2.0f : std::sin(anim * 2.0f) * 0.8f;
        float lunge = attack >= 0.0f ? std::sin(std::min(1.0f, attack) * kPi) : 0.0f;

        r.ellipse(0, 1, 22, 5, rgb(0, 0, 0, 80), 20);
        leg(r, 4, 40, -0.4f * s + 0.05f, 0.25f + 0.3f * std::max(0.0f, s), 20, 20, 8, pantsD, foot, 7);

        r.push();
        r.translate(2, 38 + bob);
        r.rotate(0.32f + lunge * 0.28f);

        arm(r, 5, 32, -1.15f + 0.15f * std::sin(anim * 3.1f), -0.25f, 16, 16, 6, skinD, skinD, 3.5f);

        r.roundedRect(-11, 0, 22, 36, 6, shirt);
        r.triangle({ -11, 3 }, { -5, 3 }, { -9, -6 }, shirt);
        r.triangle({ -3, 3 }, { 4, 3 }, { 1, -7 }, shirt);
        r.triangle({ 5, 3 }, { 11, 3 }, { 9, -5 }, shirt);
        r.ellipse(-2, 20, 6, 8, skinD, 14);
        for (int i = 0; i < 3; ++i) r.rect(-6, 15 + i * 4.0f, 8, 1.6f, rgb(196, 204, 176));
        r.line({ 4, 6 }, { 9, 28 }, 1.6f, shirtD);
        r.rect(-11, 28, 22, 3, shirtD);

        r.rect(-5, 33, 9, 9, skin);
        float sac = 0.3f + 0.7f * charge;
        r.ellipse(-9, 37, 6 + sac * 6, 5 + sac * 5, acid, 18);
        r.ellipse(-12, 40, 2 + sac * 2, 1.5f + sac * 1.5f, rgb(232, 246, 168, 190), 10);
        r.line({ -6, 33 }, { -14, 40 }, 1.0f, rgb(120, 150, 50));

        r.push();
        r.translate(-4, 52);
        r.rotate(-0.12f + lunge * 0.22f);
        r.circle(0, 0, 12, skin, 24);
        r.pie(1, 1, 12.5f, 0.1f, 2.2f, hair, 16);
        r.line({ 6, 9 }, { 13, -4 }, 1.6f, hair);
        r.line({ 9, 6 }, { 15, -7 }, 1.3f, hair);
        r.line({ 2, 11 }, { 7, -2 }, 1.3f, hair);
        r.circle(-6.5f, 2, 3.6f, rgb(232, 242, 96), 12);
        r.circle(-7.5f, 2, 1.5f, rgb(40, 30, 20), 8);
        r.circle(-0.5f, 3, 2.4f, rgb(40, 35, 35), 10);
        r.ellipse(-3, 6, 3, 1.2f, skinD, 8);
        float open = 2.0f + lunge * 6.0f;
        r.ellipse(-8, -6, 4.5f, open, rgb(52, 20, 26), 14);
        r.rect(-11, -6 + open - 1.6f, 6, 1.6f, rgb(222, 216, 190));
        float drip = 4.0f + charge * 4.0f;
        r.line({ -9, -6 - open }, { -10, -6 - open - drip }, 1.6f, rgb(176, 220, 70, 220));
        r.circle(-10, -6 - open - drip, 1.9f, acid, 8);
        r.pop();

        arm(r, -4, 31, -1.35f + 0.12f * std::sin(anim * 3.0f + 1.0f) - lunge * 0.3f, -0.3f, 16, 16, 6, skin, skin, 3.8f);
        r.pop();

        leg(r, -3, 40, 0.4f * s + 0.05f, 0.25f + 0.3f * std::max(0.0f, -s), 20, 20, 8, pants, foot, 7);
    }

    void drawRoller(Renderer2D& r, float anim, bool walking, float attack, float charge)
    {
        Color skin = rgb(118, 140, 100), skinD = rgb(88, 106, 74);
        Color shirt = rgb(168, 152, 118), shirtD = rgb(130, 114, 84);
        Color denim = rgb(62, 84, 118), denimD = rgb(44, 62, 90);
        Color sack = rgb(128, 96, 58), sackD = rgb(94, 68, 40), foot = rgb(56, 42, 34);

        float s = walking ? std::sin(anim * 4.2f) : 0.0f;
        float bob = walking ? std::fabs(std::cos(anim * 4.2f)) * 2.5f : std::sin(anim * 1.8f) * 0.8f;
        float lunge = attack >= 0.0f ? std::sin(std::min(1.0f, attack) * kPi) : 0.0f;

        float armA;
        bool holding;
        if (attack >= 0.0f && attack < 1.0f) { armA = 1.4f - 2.6f * attack; holding = false; }
        else if (charge > 0.8f)              { armA = -0.2f + 1.6f * (charge - 0.8f) / 0.2f; holding = true; }
        else                                 { armA = -0.2f + 0.15f * s; holding = true; }

        r.ellipse(2, 1, 28, 6, rgb(0, 0, 0, 80), 20);
        leg(r, 8, 30, -0.35f * s, 0.35f + 0.3f * std::max(0.0f, s), 15, 15, 11, denimD, foot, 8);

        r.push();
        r.translate(22, 60 + bob);
        r.rotate(-0.15f + 0.05f * s);
        r.ellipse(0, 0, 15, 19, sack, 22);
        r.ellipse(4, -6, 9, 10, sackD, 16);
        r.ellipse(2, -3, 10, 11, sack, 16);
        r.line({ -8, -8 }, { 8, 4 }, 1.3f, sackD);
        r.line({ -6, 8 }, { 9, -8 }, 1.3f, sackD);
        skull(r, -4, 19, 6, 0.3f);
        skull(r, 6, 17, 5, -0.4f);
        r.roundedRect(-8, 13, 16, 5, 2, sackD);
        r.pop();

        r.push();
        r.translate(0, 30 + bob);
        r.rotate(0.18f + lunge * 0.15f);

        r.ellipse(0, 22, 22, 21, shirt, 26);
        r.ellipse(-5, 15, 8, 6, shirtD, 14);
        r.ellipse(6, 30, 4, 3, shirtD, 10);
        r.roundedRect(-20, 0, 40, 18, 5, denim);
        r.rect(-14, 17, 28, 12, denim);
        r.rect(-14, 27, 4, 16, denimD);
        r.rect(8, 27, 4, 16, denimD);
        r.circle(-12, 28, 2, rgb(206, 182, 90), 8);
        r.circle(10, 28, 2, rgb(206, 182, 90), 8);
        r.rect(-3, 4, 6, 8, denimD);
        r.line({ 12, 40 }, { 24, 24 }, 3.5f, sackD);

        r.push();
        r.translate(-12, 46);
        r.rotate(-0.05f + lunge * 0.1f);
        r.roundedRect(-12, -9, 22, 20, 6, skin);
        r.rect(-15, -10, 13, 8, skinD);
        for (int i = 0; i < 3; ++i) r.rect(-14 + i * 4.0f, -3.5f, 2.6f, 3, rgb(226, 220, 196));
        r.circle(-6, 4, 4.2f, rgb(236, 232, 202), 12);
        r.circle(-7, 4, 1.7f, rgb(150, 30, 30), 8);
        r.line({ 0, 5 }, { 5, 4 }, 1.6f, rgb(40, 35, 30));
        r.line({ -1, 9 }, { 8, 7 }, 1.2f, rgb(70, 40, 40));
        for (int i = 0; i < 3; ++i) r.line({ 1 + i * 3.0f, 10 }, { 1 + i * 3.0f, 6.5f }, 1.0f, rgb(70, 40, 40));
        r.roundedRect(-12, 8, 22, 7, 3, rgb(152, 50, 40));
        r.rect(-19, 8, 9, 2.5f, rgb(118, 38, 30));
        r.pop();

        r.push();
        r.translate(-10, 36);
        r.rotate(armA);
        r.roundedRect(-4, -15, 8, 17, 3.5f, skin);
        r.translate(0, -15);
        r.rotate(-0.35f);
        r.roundedRect(-3.6f, -15, 7.2f, 17, 3.2f, skin);
        r.circle(0, -15, 5, skin, 12);
        if (holding) skull(r, 0, -21, 8, 0.2f);
        r.pop();

        r.pop();
        leg(r, -6, 30, 0.35f * s, 0.35f + 0.3f * std::max(0.0f, -s), 15, 15, 11, denim, foot, 8);
    }

    void drawBrute(Renderer2D& r, float anim, bool walking, float attack, float charge)
    {
        Color skin = rgb(104, 128, 84), skinD = rgb(78, 98, 62);
        Color armour = rgb(74, 80, 92), plate = rgb(112, 118, 132), plateD = rgb(58, 62, 72);
        Color pants = rgb(70, 58, 50), pantsD = rgb(52, 42, 36), boot = rgb(36, 32, 32);
        Color wood = rgb(112, 76, 44), woodD = rgb(82, 54, 30), metal = rgb(184, 188, 196);

        float s = walking ? std::sin(anim * 8.5f) : 0.0f;
        float bob = walking ? std::fabs(std::cos(anim * 8.5f)) * 3.5f : std::sin(anim * 1.6f) * 1.0f;
        float lean = walking ? 0.22f : 0.08f;

        float clubA;
        if (attack >= 0.0f && attack < 1.0f)
        {
            if (attack < 0.35f) clubA = 2.7f - 3.9f * (attack / 0.35f);
            else                clubA = -1.2f + 3.6f * ((attack - 0.35f) / 0.65f);
        }
        else clubA = 2.4f + 0.35f * charge + 0.08f * s;

        r.ellipse(0, 1, 34, 7, rgb(0, 0, 0, 90), 24);
        leg(r, 10, 44, -0.55f * s, 0.3f + 0.5f * std::max(0.0f, s), 22, 22, 15, pantsD, boot, 11);

        r.push();
        r.translate(2, 42 + bob);
        r.rotate(lean);

        arm(r, 12, 40, -0.6f - 0.45f * s, -0.4f, 22, 20, 12, skinD, skinD, 7.5f);

        r.roundedRect(-26, 0, 52, 48, 14, skin);
        r.roundedRect(-24, 6, 46, 36, 8, armour);
        r.roundedRect(-21, 25, 19, 14, 3, plate);
        r.roundedRect(1, 25, 19, 14, 3, plate);
        r.roundedRect(-21, 9, 19, 14, 3, plate);
        r.roundedRect(1, 9, 19, 14, 3, plate);
        for (int i = 0; i < 2; ++i)
            for (int j = 0; j < 2; ++j)
            {
                float px = -19 + i * 22.0f, py = 11 + j * 16.0f;
                r.circle(px + 1.5f, py + 1.5f, 1.4f, plateD, 6);
                r.circle(px + 15.5f, py + 10.5f, 1.4f, plateD, 6);
            }
        r.line({ -17, 36 }, { -9, 29 }, 1.3f, plateD);
        r.line({ 6, 20 }, { 15, 12 }, 1.3f, plateD);
        r.rect(-26, 0, 52, 7, rgb(52, 40, 32));
        r.rect(-6, 0, 11, 7, metal);
        r.rect(-3, 2, 5, 3, rgb(52, 40, 32));
        for (int i = 0; i < 5; ++i) r.ring(-20 + i * 9.0f, -2, 2.0f, 3.8f, metal, 10);

        r.push();
        r.translate(-10, 56);
        r.circle(0, 0, 13, skin, 24);
        r.rect(-13, -10, 15, 7, skinD);
        r.triangle({ -12, -3 }, { -9, -3 }, { -10.5f, 2 }, rgb(226, 220, 196));
        r.triangle({ -5, -3 }, { -2, -3 }, { -3.5f, 1.5f }, rgb(226, 220, 196));
        r.pie(0, 1, 14.5f, 0.0f, kPi, plate, 18);
        r.rect(-14.5f, -1, 29, 4, plateD);
        r.rect(-14, 3, 17, 4.5f, rgb(20, 18, 20));
        float glow = 0.7f + 0.3f * std::sin(anim * 6.0f);
        r.circle(-10, 5.2f, 3.5f, rgb(255, 60, 40, (int)(90 * glow)), 10);
        r.circle(-4, 5.2f, 3.5f, rgb(255, 60, 40, (int)(90 * glow)), 10);
        r.circle(-10, 5.2f, 1.6f, rgb(255, 90, 60), 8);
        r.circle(-4, 5.2f, 1.6f, rgb(255, 90, 60), 8);
        r.circle(7, 10, 1.5f, plateD, 6);
        r.circle(10, 6, 1.5f, plateD, 6);
        r.pop();

        r.push();
        r.translate(-18, 44);
        r.ellipse(0, 0, 15, 10, plate, 20);
        r.ellipse(1, -3, 12, 5, plateD, 16);
        r.triangle({ -11, 5 }, { -6, 6 }, { -9, 16 }, metal);
        r.triangle({ -3, 8 }, { 2, 8 }, { -0.5f, 19 }, metal);
        r.triangle({ 5, 7 }, { 10, 5 }, { 9, 15 }, metal);
        r.pop();

        r.push();
        r.translate(-16, 40);
        r.rotate(clubA);
        r.roundedRect(-7, -22, 14, 24, 6, skin);
        r.translate(0, -22);
        r.rotate(-0.5f);
        r.roundedRect(-6, -20, 12, 22, 5, skin);
        r.translate(0, -20);
        r.rect(-3.5f, -44, 7, 46, wood);
        r.roundedRect(-8.5f, -66, 17, 28, 6, wood);
        r.rect(-8.5f, -44, 17, 3.5f, woodD);
        r.rect(-8.5f, -58, 17, 3.5f, woodD);
        for (int i = 0; i < 3; ++i)
        {
            float ny = -46 - i * 8.0f;
            r.triangle({ -8.5f, ny }, { -8.5f, ny - 3 }, { -14, ny - 1.5f }, metal);
            r.triangle({ 8.5f, ny }, { 8.5f, ny - 3 }, { 14, ny - 1.5f }, metal);
        }
        r.triangle({ -3, -66 }, { 3, -66 }, { 0, -72 }, metal);
        r.circle(0, 0, 8, skinD, 14);
        r.pop();

        r.pop();
        leg(r, -8, 44, 0.55f * s, 0.3f + 0.5f * std::max(0.0f, -s), 22, 22, 15, pants, boot, 11);
    }

    void fire(World& w, const Enemy& e)
    {
        Projectile pr;
        pr.fromPlayer = false;
        pr.lane = e.lane;
        pr.damage = ENEMY_DAMAGE;
        float ground = laneGroundY(e.lane);
        if (e.type == EnemyType::UpperRanged)
        {
            pr.kind = ENEMY_PROJ_SPIT;
            pr.height = AttackHeight::High;
            pr.x = e.x - 22.0f;
            pr.y = ground + 78.0f;
            pr.vx = -kSpitSpeed;
        }
        else
        {
            pr.kind = ENEMY_PROJ_SKULL;
            pr.height = AttackHeight::Low;
            pr.x = e.x - 28.0f;
            pr.y = ground + 9.0f;
            pr.vx = -kSkullSpeed;
        }
        w.projectiles.push_back(pr);
        Audio::play(Sfx::EnemyAttack);
    }
}

namespace Enemies
{
    int maxHpFor(EnemyType t)
    {
        switch (t)
        {
        case EnemyType::Melee:       return HARD_ENEMY_HP;
        case EnemyType::LowerRanged: return MEDIUM_ENEMY_HP;
        default:                     return EASY_ENEMY_HP;
        }
    }

    float attackInterval(EnemyType t)
    {
        switch (t)
        {
        case EnemyType::Melee:       return 1.4f;
        case EnemyType::LowerRanged: return 3.6f;
        default:                     return 3.2f;
        }
    }

    void spawn(World& w, EnemyType t, int lane)
    {
        Enemy e;
        e.type = t;
        e.lane = lane;
        e.x = ENEMY_SPAWN_X;
        e.hp = e.maxHp = maxHpFor(t);
        switch (t)
        {
        case EnemyType::UpperRanged: e.halfWidth = 20.0f; e.height = 96.0f;  break;
        case EnemyType::LowerRanged: e.halfWidth = 26.0f; e.height = 90.0f;  break;
        case EnemyType::Melee:       e.halfWidth = 32.0f; e.height = 114.0f; break;
        default: break;
        }
        e.attackTimer = randRange(0.8f, 1.4f);
        e.anim = randRange(0.0f, 3.0f);
        w.enemies.push_back(e);
    }

    void update(World& w, float dt)
    {
        for (Enemy& e : w.enemies)
        {
            if (!e.alive) continue;
            e.anim += dt;
            e.spawnT += dt;
            e.hitFlash = std::max(0.0f, e.hitFlash - dt);

            e.engaged = false;
            for (const Player& p : w.players)
                if (p.alive && p.lane == e.lane && std::fabs(e.x - p.x) < p.halfWidth + e.halfWidth)
                    e.engaged = true;

            if (!e.engaged)
                e.x -= speedFor(e.type) * dt;

            float prevAnim = e.attackAnim;
            e.attackAnim += dt;
            e.attackTimer -= dt;
            if (e.attackTimer <= 0.0f)
            {
                e.attackTimer += attackInterval(e.type);
                e.attackAnim = 0.0f;
                prevAnim = -1.0f;
                if (e.type == EnemyType::Melee) { if (e.engaged) Audio::play(Sfx::EnemyAttack); }
                else fire(w, e);
            }
            if (e.type == EnemyType::Melee && prevAnim < kBruteHitAt && e.attackAnim >= kBruteHitAt)
                e.strikeNow = true;
        }
    }

    void drawModel(Renderer2D& r, EnemyType t, float x, float groundY, float anim,
                   bool walking, float attack, float charge, float damage)
    {
        r.push();
        r.translate(x, groundY);
        r.setDamageTint(damage);
        switch (t)
        {
        case EnemyType::UpperRanged: drawSpitter(r, anim, walking, attack, charge); break;
        case EnemyType::LowerRanged: drawRoller(r, anim, walking, attack, charge);  break;
        case EnemyType::Melee:       drawBrute(r, anim, walking, attack, charge);   break;
        default: break;
        }
        r.pop();
    }

    void draw(Renderer2D& r, const World&, const Enemy& e)
    {
        float ground = laneGroundY(e.lane);

        if (e.spawnT < 0.7f)
        {
            float k = e.spawnT / 0.7f;
            for (int i = 0; i < 7; ++i)
            {
                float a = 0.35f + i * 0.4f;
                float d = 10.0f + 34.0f * k;
                r.circle(e.x + std::cos(a) * d, ground + std::sin(a) * d * 0.6f - 30.0f * k * k + 6.0f,
                         4.5f * (1.0f - k) + 1.0f, rgb(94, 72, 50, (int)(220 * (1.0f - k))), 10);
            }
            r.ellipse(e.x, ground + 2, 26 * (1.0f - k * 0.5f), 6, rgb(70, 52, 36, (int)(200 * (1.0f - k))), 18);
        }

        float dmg = 1.0f - (float)e.hp / (float)e.maxHp;
        if (e.hitFlash > 0.0f) dmg = std::max(dmg, 0.6f);
        float dur = attackDuration(e.type);
        float attack = e.attackAnim < dur ? e.attackAnim / dur : -1.0f;
        float charge = std::clamp(1.0f - e.attackTimer / attackInterval(e.type), 0.0f, 1.0f);
        float recoil = e.hitFlash > 0.0f ? 3.0f : 0.0f;

        r.push();
        r.setAlpha(std::min(1.0f, e.spawnT / 0.5f));
        drawModel(r, e.type, e.x + recoil, ground, e.anim, !e.engaged, attack, charge, dmg);
        r.pop();
    }

    void drawProjectile(Renderer2D& r, const Projectile& pr)
    {
        bool impact = (pr.kind & ENEMY_PROJ_IMPACT) != 0;
        int kind = pr.kind & 0xFF;

        if (kind == ENEMY_PROJ_SPIT)
        {
            if (impact)
            {
                float k = std::min(1.0f, pr.age / 0.35f);
                int a = (int)(220 * (1.0f - k));
                r.ring(pr.x, pr.y, 3 + 14 * k, 6 + 16 * k, rgb(176, 214, 72, a), 18);
                for (int i = 0; i < 6; ++i)
                {
                    float ang = i * 1.05f + 0.4f;
                    r.circle(pr.x + std::cos(ang) * 22 * k, pr.y + std::sin(ang) * 16 * k - 30 * k * k,
                             3.0f * (1.0f - k) + 0.8f, rgb(176, 214, 72, a), 8);
                }
                return;
            }
            for (int i = 1; i <= 3; ++i)
                r.circle(pr.x + i * 8.0f, pr.y + std::sin(pr.age * 14.0f - i) * 2.0f, 4.5f - i, rgb(176, 214, 72, 200 - i * 50), 10);
            r.push();
            r.translate(pr.x, pr.y);
            r.rotate(std::atan2(pr.vy, pr.vx) + kPi);
            r.circle(0, 0, 10, rgb(176, 214, 72, 50), 16);
            r.ellipse(0, 0, 9, 6, rgb(150, 196, 56), 16);
            r.ellipse(-1, 1, 6, 3.5f, rgb(196, 232, 96), 12);
            r.circle(-3, 2, 1.6f, rgb(240, 255, 200), 8);
            r.pop();
            return;
        }

        if (impact)
        {
            float k = std::min(1.0f, pr.age / 0.35f);
            int a = (int)(255 * (1.0f - k));
            for (int i = 0; i < 6; ++i)
            {
                float ang = 0.3f + i * 0.5f;
                float px = pr.x + std::cos(ang) * 30 * k;
                float py = pr.y + std::sin(ang) * 36 * k - 60 * k * k;
                r.push();
                r.translate(px, py);
                r.rotate(k * 8.0f + i);
                r.triangle({ -3, -2 }, { 3, -2 }, { 0, 3 }, rgb(234, 228, 206, a));
                r.pop();
            }
            r.circle(pr.x, pr.y, 10 * (1.0f - k) + 2, rgb(150, 130, 110, a / 2), 12);
            return;
        }
        for (int i = 1; i <= 3; ++i)
            r.circle(pr.x + i * 10.0f, pr.y - 5 + i * 1.5f, 3.5f + i * 0.8f, rgb(130, 110, 86, 110 - i * 30), 10);
        skull(r, pr.x, pr.y, 9, pr.age * 9.0f);
    }

    void damage(World& w, Enemy& e, int amount)
    {
        if (!e.alive) return;
        e.hp -= amount;
        e.hitFlash = 0.15f;
        Audio::play(Sfx::Hit);
        if (e.hp <= 0)
        {
            e.hp = 0;
            e.alive = false;
            Audio::play(Sfx::EnemyDeath);
            Collectibles::spawn(w, e.lane, e.x, COIN_DROP_VALUE);
        }
    }
}

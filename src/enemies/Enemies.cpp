#include "Enemies.h"
#include "../game/World.h"
#include "../game/Collectibles.h"
#include "../core/Renderer2D.h"
#include "../core/Audio.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

static const float PI = 3.14159265f;

static Color rgb(int r, int g, int b, int a = 255)
{
    return Colors::rgb(r, g, b, a);
}

static float randomFloat(float a, float b)
{
    return a + (b - a) * (rand() / (float)RAND_MAX);
}

static float walkSpeed(EnemyType t)
{
    if (t == EnemyType::Melee) return 72;
    if (t == EnemyType::LowerRanged) return 26;
    return 30;
}

static float attackDelay(EnemyType t)
{
    if (t == EnemyType::Melee) return 1.4f;
    if (t == EnemyType::LowerRanged) return 3.6f;
    return 3.2f;
}

static float attackLength(EnemyType t)
{
    if (t == EnemyType::Melee) return 0.55f;
    if (t == EnemyType::LowerRanged) return 0.45f;
    return 0.4f;
}

static void drawLeg(Renderer2D& r, float hipX, float hipY, float thighAngle, float kneeAngle,
                    float length, float thickness, Color color, Color shoe, float shoeLength)
{
    r.push();
    r.translate(hipX, hipY);
    r.rotate(thighAngle);
    r.roundedRect(-thickness / 2, -length, thickness, length + 2, thickness * 0.45f, color);
    r.translate(0, -length);
    r.rotate(kneeAngle);
    r.roundedRect(-thickness * 0.45f, -length, thickness * 0.9f, length + 2, thickness * 0.4f, color);
    r.translate(0, -length);
    r.rotate(-(thighAngle + kneeAngle));
    r.roundedRect(-thickness / 2 - shoeLength, -3, thickness + shoeLength + 2, 7, 3, shoe);
    r.pop();
}

static void drawArm(Renderer2D& r, float shoulderX, float shoulderY, float angle, float elbow,
                    float length, float thickness, Color color, float handSize)
{
    r.push();
    r.translate(shoulderX, shoulderY);
    r.rotate(angle);
    r.roundedRect(-thickness / 2, -length, thickness, length + 2, thickness * 0.45f, color);
    r.translate(0, -length);
    r.rotate(elbow);
    r.roundedRect(-thickness * 0.45f, -length, thickness * 0.9f, length + 2, thickness * 0.4f, color);
    r.circle(0, -length, handSize, color, 12);
    r.line({ 0, -length }, { -handSize * 0.6f, -length - handSize * 1.4f }, 1.4f, rgb(220, 210, 170));
    r.line({ 0, -length }, { handSize * 0.5f, -length - handSize * 1.5f }, 1.4f, rgb(220, 210, 170));
    r.pop();
}

static void drawSkull(Renderer2D& r, float x, float y, float size, float angle)
{
    Color bone = rgb(234, 228, 206);
    Color boneDark = rgb(196, 188, 160);
    Color hole = rgb(40, 30, 30);

    r.push();
    r.translate(x, y);
    r.rotate(angle);
    r.circle(0, 0, size, bone, 18);
    r.roundedRect(-size * 0.55f, -size * 1.05f, size * 1.1f, size * 0.6f, size * 0.2f, boneDark);
    r.circle(-size * 0.4f, size * 0.05f, size * 0.28f, hole, 10);
    r.circle(size * 0.4f, size * 0.05f, size * 0.28f, hole, 10);
    r.triangle({ -size * 0.12f, -size * 0.3f }, { size * 0.12f, -size * 0.3f }, { 0, -size * 0.08f }, hole);
    for (int i = 0; i < 3; i++)
        r.rect(-size * 0.4f + i * size * 0.3f, -size * 0.9f, size * 0.08f + 0.6f, size * 0.35f, hole);
    r.pop();
}

static void drawSpitter(Renderer2D& r, float anim, bool walking, float attack)
{
    Color skin = rgb(132, 163, 100);
    Color skinDark = rgb(96, 124, 74);
    Color shirt = rgb(112, 86, 122);
    Color shirtDark = rgb(80, 58, 90);
    Color pants = rgb(70, 68, 84);
    Color pantsDark = rgb(52, 50, 64);
    Color shoe = rgb(58, 46, 40);
    Color hair = rgb(58, 50, 44);
    Color acid = rgb(176, 214, 72);

    float step = walking ? sin(anim * 5) : 0;
    float bob = walking ? fabs(cos(anim * 5)) * 2 : sin(anim * 2) * 0.8f;
    float lunge = attack >= 0 ? sin(attack * PI) : 0;

    r.ellipse(0, 1, 22, 5, rgb(0, 0, 0, 80), 20);
    drawLeg(r, 4, 40, -0.4f * step + 0.05f, 0.25f + 0.3f * std::max(0.0f, step), 20, 8, pantsDark, shoe, 7);

    r.push();
    r.translate(2, 38 + bob);
    r.rotate(0.32f + lunge * 0.28f);

    drawArm(r, 5, 32, -1.15f + 0.15f * sin(anim * 3.1f), -0.25f, 16, 6, skinDark, 3.5f);

    r.roundedRect(-11, 0, 22, 36, 6, shirt);
    r.triangle({ -11, 3 }, { -5, 3 }, { -9, -6 }, shirt);
    r.triangle({ -3, 3 }, { 4, 3 }, { 1, -7 }, shirt);
    r.triangle({ 5, 3 }, { 11, 3 }, { 9, -5 }, shirt);
    r.ellipse(-2, 20, 6, 8, skinDark, 14);
    for (int i = 0; i < 3; i++)
        r.rect(-6, 15 + i * 4.0f, 8, 1.6f, rgb(196, 204, 176));
    r.line({ 4, 6 }, { 9, 28 }, 1.6f, shirtDark);
    r.rect(-11, 28, 22, 3, shirtDark);

    r.rect(-5, 33, 9, 9, skin);
    float sac = 0.5f + 0.2f * sin(anim * 3) + 0.4f * lunge;
    r.ellipse(-9, 37, 6 + sac * 6, 5 + sac * 5, acid, 18);
    r.ellipse(-12, 40, 2 + sac * 2, 1.5f + sac * 1.5f, rgb(232, 246, 168, 190), 10);

    r.push();
    r.translate(-4, 52);
    r.rotate(-0.12f + lunge * 0.22f);
    r.circle(0, 0, 12, skin, 24);
    r.pie(1, 1, 12.5f, 0.1f, 2.2f, hair, 16);
    r.line({ 6, 9 }, { 13, -4 }, 1.6f, hair);
    r.line({ 9, 6 }, { 15, -7 }, 1.3f, hair);
    r.circle(-6.5f, 2, 3.6f, rgb(232, 242, 96), 12);
    r.circle(-7.5f, 2, 1.5f, rgb(40, 30, 20), 8);
    r.circle(-0.5f, 3, 2.4f, rgb(40, 35, 35), 10);
    float mouth = 2 + lunge * 6;
    r.ellipse(-8, -6, 4.5f, mouth, rgb(52, 20, 26), 14);
    r.rect(-11, -6 + mouth - 1.6f, 6, 1.6f, rgb(222, 216, 190));
    r.line({ -9, -6 - mouth }, { -10, -12 - mouth }, 1.6f, acid);
    r.pop();

    drawArm(r, -4, 31, -1.35f + 0.12f * sin(anim * 3 + 1) - lunge * 0.3f, -0.3f, 16, 6, skin, 3.8f);
    r.pop();

    drawLeg(r, -3, 40, 0.4f * step + 0.05f, 0.25f + 0.3f * std::max(0.0f, -step), 20, 8, pants, shoe, 7);
}

static void drawBoneRoller(Renderer2D& r, float anim, bool walking, float attack)
{
    Color skin = rgb(118, 140, 100);
    Color skinDark = rgb(88, 106, 74);
    Color shirt = rgb(168, 152, 118);
    Color shirtDark = rgb(130, 114, 84);
    Color denim = rgb(62, 84, 118);
    Color denimDark = rgb(44, 62, 90);
    Color sack = rgb(128, 96, 58);
    Color sackDark = rgb(94, 68, 40);
    Color shoe = rgb(56, 42, 34);

    float step = walking ? sin(anim * 4.2f) : 0;
    float bob = walking ? fabs(cos(anim * 4.2f)) * 2.5f : sin(anim * 1.8f) * 0.8f;
    float lunge = attack >= 0 ? sin(attack * PI) : 0;

    float armAngle = -0.2f + 0.15f * step;
    bool holdingSkull = true;
    if (attack >= 0)
    {
        armAngle = 1.4f - 2.6f * attack;
        holdingSkull = false;
    }

    r.ellipse(2, 1, 28, 6, rgb(0, 0, 0, 80), 20);
    drawLeg(r, 8, 30, -0.35f * step, 0.35f + 0.3f * std::max(0.0f, step), 15, 11, denimDark, shoe, 8);

    r.push();
    r.translate(22, 60 + bob);
    r.rotate(-0.15f + 0.05f * step);
    r.ellipse(0, 0, 15, 19, sack, 22);
    r.ellipse(4, -6, 9, 10, sackDark, 16);
    r.ellipse(2, -3, 10, 11, sack, 16);
    r.line({ -8, -8 }, { 8, 4 }, 1.3f, sackDark);
    r.line({ -6, 8 }, { 9, -8 }, 1.3f, sackDark);
    drawSkull(r, -4, 19, 6, 0.3f);
    drawSkull(r, 6, 17, 5, -0.4f);
    r.roundedRect(-8, 13, 16, 5, 2, sackDark);
    r.pop();

    r.push();
    r.translate(0, 30 + bob);
    r.rotate(0.18f + lunge * 0.15f);

    r.ellipse(0, 22, 22, 21, shirt, 26);
    r.ellipse(-5, 15, 8, 6, shirtDark, 14);
    r.roundedRect(-20, 0, 40, 18, 5, denim);
    r.rect(-14, 17, 28, 12, denim);
    r.rect(-14, 27, 4, 16, denimDark);
    r.rect(8, 27, 4, 16, denimDark);
    r.circle(-12, 28, 2, rgb(206, 182, 90), 8);
    r.circle(10, 28, 2, rgb(206, 182, 90), 8);
    r.line({ 12, 40 }, { 24, 24 }, 3.5f, sackDark);

    r.push();
    r.translate(-12, 46);
    r.roundedRect(-12, -9, 22, 20, 6, skin);
    r.rect(-15, -10, 13, 8, skinDark);
    for (int i = 0; i < 3; i++)
        r.rect(-14 + i * 4.0f, -3.5f, 2.6f, 3, rgb(226, 220, 196));
    r.circle(-6, 4, 4.2f, rgb(236, 232, 202), 12);
    r.circle(-7, 4, 1.7f, rgb(150, 30, 30), 8);
    r.line({ 0, 5 }, { 5, 4 }, 1.6f, rgb(40, 35, 30));
    r.line({ -1, 9 }, { 8, 7 }, 1.2f, rgb(70, 40, 40));
    r.roundedRect(-12, 8, 22, 7, 3, rgb(152, 50, 40));
    r.rect(-19, 8, 9, 2.5f, rgb(118, 38, 30));
    r.pop();

    r.push();
    r.translate(-10, 36);
    r.rotate(armAngle);
    r.roundedRect(-4, -15, 8, 17, 3.5f, skin);
    r.translate(0, -15);
    r.rotate(-0.35f);
    r.roundedRect(-3.6f, -15, 7.2f, 17, 3.2f, skin);
    r.circle(0, -15, 5, skin, 12);
    if (holdingSkull) drawSkull(r, 0, -21, 8, 0.2f);
    r.pop();

    r.pop();
    drawLeg(r, -6, 30, 0.35f * step, 0.35f + 0.3f * std::max(0.0f, -step), 15, 11, denim, shoe, 8);
}

static void drawBrute(Renderer2D& r, float anim, bool walking, float attack)
{
    Color skin = rgb(104, 128, 84);
    Color skinDark = rgb(78, 98, 62);
    Color armour = rgb(74, 80, 92);
    Color plate = rgb(112, 118, 132);
    Color plateDark = rgb(58, 62, 72);
    Color pants = rgb(70, 58, 50);
    Color pantsDark = rgb(52, 42, 36);
    Color boot = rgb(36, 32, 32);
    Color wood = rgb(112, 76, 44);
    Color woodDark = rgb(82, 54, 30);
    Color metal = rgb(184, 188, 196);

    float step = walking ? sin(anim * 8.5f) : 0;
    float bob = walking ? fabs(cos(anim * 8.5f)) * 3.5f : sin(anim * 1.6f);
    float lean = walking ? 0.22f : 0.08f;

    float clubAngle = 2.4f + 0.08f * step;
    if (attack >= 0)
    {
        if (attack < 0.35f) clubAngle = 2.7f - 3.9f * (attack / 0.35f);
        else                clubAngle = -1.2f + 3.6f * ((attack - 0.35f) / 0.65f);
    }

    r.ellipse(0, 1, 34, 7, rgb(0, 0, 0, 90), 24);
    drawLeg(r, 10, 44, -0.55f * step, 0.3f + 0.5f * std::max(0.0f, step), 22, 15, pantsDark, boot, 11);

    r.push();
    r.translate(2, 42 + bob);
    r.rotate(lean);

    drawArm(r, 12, 40, -0.6f - 0.45f * step, -0.4f, 21, 12, skinDark, 7.5f);

    r.roundedRect(-26, 0, 52, 48, 14, skin);
    r.roundedRect(-24, 6, 46, 36, 8, armour);
    r.roundedRect(-21, 25, 19, 14, 3, plate);
    r.roundedRect(1, 25, 19, 14, 3, plate);
    r.roundedRect(-21, 9, 19, 14, 3, plate);
    r.roundedRect(1, 9, 19, 14, 3, plate);
    r.line({ -17, 36 }, { -9, 29 }, 1.3f, plateDark);
    r.line({ 6, 20 }, { 15, 12 }, 1.3f, plateDark);
    r.rect(-26, 0, 52, 7, rgb(52, 40, 32));
    r.rect(-6, 0, 11, 7, metal);
    for (int i = 0; i < 5; i++)
        r.ring(-20 + i * 9.0f, -2, 2, 3.8f, metal, 10);

    r.push();
    r.translate(-10, 56);
    r.circle(0, 0, 13, skin, 24);
    r.rect(-13, -10, 15, 7, skinDark);
    r.triangle({ -12, -3 }, { -9, -3 }, { -10.5f, 2 }, rgb(226, 220, 196));
    r.triangle({ -5, -3 }, { -2, -3 }, { -3.5f, 1.5f }, rgb(226, 220, 196));
    r.pie(0, 1, 14.5f, 0, PI, plate, 18);
    r.rect(-14.5f, -1, 29, 4, plateDark);
    r.rect(-14, 3, 17, 4.5f, rgb(20, 18, 20));
    r.circle(-10, 5.2f, 3.5f, rgb(255, 60, 40, 80), 10);
    r.circle(-4, 5.2f, 3.5f, rgb(255, 60, 40, 80), 10);
    r.circle(-10, 5.2f, 1.6f, rgb(255, 90, 60), 8);
    r.circle(-4, 5.2f, 1.6f, rgb(255, 90, 60), 8);
    r.pop();

    r.push();
    r.translate(-18, 44);
    r.ellipse(0, 0, 15, 10, plate, 20);
    r.ellipse(1, -3, 12, 5, plateDark, 16);
    r.triangle({ -11, 5 }, { -6, 6 }, { -9, 16 }, metal);
    r.triangle({ -3, 8 }, { 2, 8 }, { -0.5f, 19 }, metal);
    r.triangle({ 5, 7 }, { 10, 5 }, { 9, 15 }, metal);
    r.pop();

    r.push();
    r.translate(-16, 40);
    r.rotate(clubAngle);
    r.roundedRect(-7, -22, 14, 24, 6, skin);
    r.translate(0, -22);
    r.rotate(-0.5f);
    r.roundedRect(-6, -20, 12, 22, 5, skin);
    r.translate(0, -20);
    r.rect(-3.5f, -44, 7, 46, wood);
    r.roundedRect(-8.5f, -66, 17, 28, 6, wood);
    r.rect(-8.5f, -44, 17, 3.5f, woodDark);
    r.rect(-8.5f, -58, 17, 3.5f, woodDark);
    for (int i = 0; i < 3; i++)
    {
        float y = -46 - i * 8.0f;
        r.triangle({ -8.5f, y }, { -8.5f, y - 3 }, { -14, y - 1.5f }, metal);
        r.triangle({ 8.5f, y }, { 8.5f, y - 3 }, { 14, y - 1.5f }, metal);
    }
    r.circle(0, 0, 8, skinDark, 14);
    r.pop();

    r.pop();
    drawLeg(r, -8, 44, 0.55f * step, 0.3f + 0.5f * std::max(0.0f, -step), 22, 15, pants, boot, 11);
}

static void shoot(World& w, const Enemy& e)
{
    Projectile p;
    p.fromPlayer = false;
    p.lane = e.lane;
    p.damage = ENEMY_DAMAGE;

    if (e.type == EnemyType::UpperRanged)
    {
        p.kind = SPIT;
        p.height = AttackHeight::High;
        p.x = e.x - 22;
        p.y = laneGroundY(e.lane) + 76;
        p.vx = -270;
    }
    else
    {
        p.kind = SKULL;
        p.height = AttackHeight::Low;
        p.x = e.x - 28;
        p.y = laneGroundY(e.lane) + 9;
        p.vx = -220;
    }

    w.projectiles.push_back(p);
    Audio::play(Sfx::EnemyAttack);
}

namespace Enemies
{
    void spawn(World& w, EnemyType t, int lane)
    {
        Enemy e;
        e.type = t;
        e.lane = lane;
        e.attackTimer = randomFloat(0.8f, 1.4f);
        e.anim = randomFloat(0, 3);

        if (t == EnemyType::UpperRanged)
        {
            e.hp = EASY_ENEMY_HP;
            e.halfWidth = 20;
            e.height = 96;
        }
        else if (t == EnemyType::LowerRanged)
        {
            e.hp = MEDIUM_ENEMY_HP;
            e.halfWidth = 26;
            e.height = 90;
        }
        else
        {
            e.hp = HARD_ENEMY_HP;
            e.halfWidth = 32;
            e.height = 114;
        }
        e.maxHp = e.hp;

        w.enemies.push_back(e);
    }

    void update(World& w, float dt)
    {
        for (Enemy& e : w.enemies)
        {
            if (!e.alive) continue;
            e.anim += dt;

            e.engaged = false;
            for (const Player& p : w.players)
                if (p.alive && p.lane == e.lane && fabs(e.x - p.x) < p.halfWidth + e.halfWidth)
                    e.engaged = true;

            if (!e.engaged)
                e.x -= walkSpeed(e.type) * dt;

            float before = e.attackAnim;
            e.attackAnim += dt;
            e.attackTimer -= dt;

            if (e.attackTimer <= 0)
            {
                e.attackTimer = attackDelay(e.type);
                e.attackAnim = 0;
                before = -1;

                if (e.type != EnemyType::Melee)
                    shoot(w, e);
                else if (e.engaged)
                    Audio::play(Sfx::EnemyAttack);
            }

            if (e.type == EnemyType::Melee && before < 0.17f && e.attackAnim >= 0.17f)
                e.strikeNow = true;
        }
    }

    void drawModel(Renderer2D& r, EnemyType t, float x, float groundY, float anim,
                   bool walking, float attack, float damage)
    {
        r.push();
        r.translate(x, groundY);
        r.setDamageTint(damage);

        if (t == EnemyType::UpperRanged)      drawSpitter(r, anim, walking, attack);
        else if (t == EnemyType::LowerRanged) drawBoneRoller(r, anim, walking, attack);
        else                                  drawBrute(r, anim, walking, attack);

        r.pop();
    }

    void draw(Renderer2D& r, const World&, const Enemy& e)
    {
        float attack = -1;
        if (e.attackAnim < attackLength(e.type))
            attack = e.attackAnim / attackLength(e.type);

        float damage = 1.0f - (float)e.hp / e.maxHp;
        drawModel(r, e.type, e.x, laneGroundY(e.lane), e.anim, !e.engaged, attack, damage);
    }

    void drawProjectile(Renderer2D& r, const Projectile& p)
    {
        if (p.kind == SKULL)
        {
            drawSkull(r, p.x, p.y, 9, p.age * 9);
            return;
        }

        r.circle(p.x, p.y, 10, rgb(176, 214, 72, 50), 16);
        r.ellipse(p.x, p.y, 9, 6, rgb(150, 196, 56), 16);
        r.ellipse(p.x - 1, p.y + 1, 6, 3.5f, rgb(196, 232, 96), 12);
        r.circle(p.x + 8, p.y + 1, 3, rgb(176, 214, 72, 150), 10);
        r.circle(p.x + 15, p.y + 2, 2, rgb(176, 214, 72, 100), 8);
    }

    void damage(World& w, Enemy& e, int amount)
    {
        if (!e.alive) return;

        e.hp -= amount;
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

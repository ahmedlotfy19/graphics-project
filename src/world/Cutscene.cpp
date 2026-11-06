#include "Cutscene.h"
#include "../Config.h"
#include "../core/Renderer2D.h"
#include "../core/Text.h"
#include "../players/Players.h"
#include "../enemies/Enemies.h"
#include "../game/Projectile.h"

#include <cmath>

using Colors::rgb;

static float g_time = 0;
const float DURATION = 17.0f;
const float GROUND = 150;
const float SIZE = 1.5f;

struct Line
{
    float start, end;
    const char* speaker;
    const char* text;
    Color color;
};

static const Line LINES[] =
{
    { 0.5f,  4.3f,  "",         "THE LAST SAFE HOUSE IN TOWN.\nINSIDE: FOOD, MEDICINE AND TWELVE SURVIVORS.", Color(0.85f, 0.85f, 0.85f, 1) },
    { 4.5f,  6.6f,  "SPITTER",  "HHRRR... FRESH MEAT IN THAT HOUSE...",          Color(0.6f, 0.9f, 0.4f, 1) },
    { 6.6f,  8.6f,  "SURVIVOR", "NOT A CHANCE. NOBODY GETS PAST THESE LANES!",   Color(0.5f, 0.75f, 1.0f, 1) },
    { 8.7f,  10.3f, "SURVIVOR", "WHOA! DUCK!",                                    Color(0.5f, 0.75f, 1.0f, 1) },
    { 10.6f, 12.4f, "SURVIVOR", "TAKE THAT!",                                     Color(0.5f, 0.75f, 1.0f, 1) },
    { 12.6f, 14.8f, "BRUTE",    "THERE ARE THOUSANDS OF US... WE ARE COMING!",    Color(0.95f, 0.4f, 0.35f, 1) },
    { 14.8f, 16.6f, "SURVIVOR", "EVERYONE TO THE LANES! DEFEND THE SAFE HOUSE!",  Color(0.5f, 0.75f, 1.0f, 1) },
};

static float between(float t, float a, float b)
{
    if (t <= a) return 0;
    if (t >= b) return 1;
    return (t - a) / (b - a);
}

static void drawStage(Renderer2D& r, float t)
{
    r.rectGradient(0, 0, (float)VIRTUAL_W, (float)VIRTUAL_H, rgb(50, 36, 66), rgb(10, 8, 22));

    for (int i = 0; i < 60; i++)
    {
        float x = std::fmod(i * 197.0f, (float)VIRTUAL_W);
        float y = 300 + std::fmod(i * 71.0f, 410.0f);
        float twinkle = 0.5f + 0.5f * std::sin(t * 2 + i);
        r.circle(x, y, 1.4f, Color(1, 1, 1, 0.3f + 0.5f * twinkle), 6);
    }

    r.circle(980, 560, 70, rgb(255, 250, 220, 30), 40);
    r.circle(980, 560, 50, rgb(236, 232, 210), 40);
    r.circle(965, 575, 10, rgb(210, 206, 186), 16);
    r.circle(1000, 545, 7, rgb(210, 206, 186), 14);

    for (int i = 0; i < 14; i++)
    {
        float w = 70 + std::fmod(i * 37.0f, 50.0f);
        float h = 60 + std::fmod(i * 53.0f, 110.0f);
        r.rect(300 + i * 75.0f, GROUND + 20, w, h, rgb(24, 20, 34));
    }

    r.rectGradient(0, 0, (float)VIRTUAL_W, GROUND + 20, rgb(60, 48, 36), rgb(78, 92, 56));
    r.rect(0, GROUND - 10, (float)VIRTUAL_W, 30, rgb(96, 76, 54));

    r.rect(20, GROUND, 220, 190, rgb(100, 68, 44));
    for (int i = 0; i < 10; i++)
        r.rect(20, GROUND + i * 19, 220, 17, i % 2 == 0 ? rgb(108, 74, 48) : rgb(98, 66, 42));
    r.triangle({ 0, GROUND + 190 }, { 260, GROUND + 190 }, { 130, GROUND + 280 }, rgb(70, 40, 34));
    r.rect(180, GROUND + 230, 20, 50, rgb(70, 60, 58));

    float flicker = 0.8f + 0.2f * std::sin(t * 7);
    r.rect(40, GROUND + 100, 60, 50, Color(1.0f, 0.72f, 0.3f, flicker));
    r.rect(68, GROUND + 100, 4, 50, rgb(50, 32, 22));
    r.line({ 36, GROUND + 98 }, { 104, GROUND + 152 }, 6, rgb(130, 92, 56));

    float doorOpen = between(t, 1.0f, 1.6f) * (1 - between(t, 3.0f, 3.5f));
    r.rect(140, GROUND, 60, 110, rgb(36, 22, 16));
    r.rect(140, GROUND, 60 * (1 - doorOpen * 0.8f), 110, rgb(76, 50, 32));

    r.roundedRect(40, GROUND + 160, 180, 26, 4, rgb(176, 150, 104));
    Text::draw(r, "SAFE HOUSE", 130, GROUND + 166, 14, rgb(110, 30, 24), Align::Center);

    for (int i = 0; i < 4; i++)
    {
        float sy = GROUND + 4 + i * 20;
        r.triangle({ 240, sy }, { 240, sy + 10 }, { 285, sy + 5 }, rgb(150, 116, 74));
    }
}

static void drawPlayer(Renderer2D& r, float x, float anim, Pose pose, float swing, bool walking, bool faceLeft)
{
    r.push();
    r.translate(x, GROUND);
    r.scale(faceLeft ? -SIZE : SIZE, SIZE);
    Players::drawModel(r, PlayerType::Melee, 0, 0, anim, pose, 0, swing, walking);
    r.pop();
}

static void drawZombie(Renderer2D& r, EnemyType type, float x, float ground, float size,
                       float anim, bool walking, float attack, float damage, float alpha)
{
    r.push();
    r.setAlpha(alpha);
    r.translate(x, ground);
    r.scale(size);
    Enemies::drawModel(r, type, 0, 0, anim, walking, attack, damage);
    r.pop();
}

static void drawSubtitle(Renderer2D& r, float t)
{
    for (const Line& line : LINES)
    {
        if (t < line.start || t > line.end) continue;

        r.roundedRect(190, 22, 900, 92, 12, Color(0, 0, 0, 0.65f));
        if (line.speaker[0] != 0)
        {
            Text::drawShadowed(r, line.speaker, 640, 86, 14, line.color, Align::Center);
            Text::drawShadowed(r, line.text, 640, 48, 20, Colors::White, Align::Center);
        }
        else
        {
            Text::drawShadowed(r, line.text, 640, 76, 18, line.color, Align::Center);
        }
    }
}

namespace Cutscene
{
    void reset()          { g_time = 0; }
    void update(float dt) { g_time += dt; }
    bool finished()       { return g_time >= DURATION; }
    float duration()      { return DURATION; }

    void draw(Renderer2D& r)
    {
        float t = g_time;
        drawStage(r, t);

        if (t > 12.3f)
        {
            float k = between(t, 12.3f, 16.0f);
            for (int i = 0; i < 6; i++)
            {
                float x = 1350 + i * 60 - k * 260;
                EnemyType type = (EnemyType)(i % 3);
                drawZombie(r, type, x, GROUND + 18, 1.0f, t + i, true, -1, 0, 0.55f);
            }
            float bruteX = 1360 - between(t, 12.3f, 13.6f) * 260;
            drawZombie(r, EnemyType::Melee, bruteX, GROUND, SIZE, t, t < 13.6f, -1, 0, 1);
        }

        float spitterX = 1350 - between(t, 3.5f, 6.4f) * 560;
        float spitterAttack = (t > 8.4f && t < 8.9f) ? between(t, 8.4f, 8.9f) : -1;
        float spitterHurt = between(t, 11.0f, 12.0f) * 0.8f;
        float spitterAlpha = 1 - between(t, 12.0f, 12.6f);
        bool spitterWalks = t > 3.5f && t < 6.4f;
        if (t > 3.5f && spitterAlpha > 0)
            drawZombie(r, EnemyType::UpperRanged, spitterX, GROUND, SIZE, t, spitterWalks,
                       spitterAttack, spitterHurt, spitterAlpha);

        if (t > 8.9f && t < 10.2f)
        {
            Projectile spit;
            spit.fromPlayer = false;
            spit.kind = SPIT;
            spit.age = t - 8.9f;
            spit.x = spitterX - 30 - spit.age * 600;
            spit.y = GROUND + 76 * SIZE + 4 * std::sin(spit.age * 14);
            r.push();
            r.translate(spit.x, spit.y);
            r.scale(SIZE);
            spit.x = 0;
            spit.y = 0;
            Enemies::drawProjectile(r, spit);
            r.pop();
        }

        if (t > 1.3f)
        {
            float x = 190 + between(t, 1.3f, 4.0f) * 240;
            bool walking = t < 4.0f;
            Pose pose = Pose::Normal;
            float swing = -1;
            bool faceLeft = false;

            if (t > 9.1f && t < 10.1f)
                pose = Pose::Ducking;

            if (t > 10.2f)
            {
                x = 430 + between(t, 10.2f, 10.9f) * 270;
                walking = t < 10.9f;
            }
            if (t > 10.9f && t < 12.1f)
                swing = std::fmod(t - 10.9f, 0.6f) / 0.6f;

            if (t > 14.8f)
            {
                x = 700 - between(t, 14.8f, 16.8f) * 560;
                walking = true;
                faceLeft = true;
            }

            drawPlayer(r, x, t, pose, swing, walking, faceLeft);
        }

        drawSubtitle(r, t);

        float fade = 1 - between(t, 0, 1.0f) + between(t, 16.0f, DURATION);
        if (fade > 0)
            r.fullscreen(Color(0, 0, 0, fade > 1 ? 1 : fade));
    }
}

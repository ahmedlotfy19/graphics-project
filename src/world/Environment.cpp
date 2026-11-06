#include "Environment.h"
#include "../Config.h"
#include "../core/Renderer2D.h"
#include "../core/Text.h"

#include <cmath>

using Colors::rgb;

static float propBase(int lane)
{
    return laneTopY(lane) - 40;
}

static void deadTree(Renderer2D& r, float x, float y)
{
    Color bark = rgb(52, 40, 34);
    r.line({ x, y }, { x + 2, y + 34 }, 5, bark);
    r.line({ x + 1, y + 20 }, { x - 12, y + 32 }, 3, bark);
    r.line({ x + 2, y + 26 }, { x + 14, y + 36 }, 3, bark);
    r.line({ x - 8, y + 28 }, { x - 14, y + 26 }, 2, bark);
    r.line({ x + 10, y + 33 }, { x + 12, y + 38 }, 2, bark);
    r.ellipse(x + 1, y, 9, 3, rgb(40, 34, 28), 12);
}

static void grave(Renderer2D& r, float x, float y)
{
    r.roundedRect(x - 9, y, 18, 24, 7, rgb(118, 118, 126));
    r.roundedRect(x - 9, y, 18, 4, 1, rgb(84, 84, 92));
    r.rect(x - 1, y + 9, 2, 10, rgb(70, 70, 78));
    r.rect(x - 4, y + 14, 8, 2, rgb(70, 70, 78));
    r.ellipse(x, y, 12, 3, rgb(60, 50, 38), 12);
}

static void wreckedCar(Renderer2D& r, float x, float y)
{
    Color body = rgb(120, 52, 42);
    Color rust = rgb(92, 56, 36);
    r.roundedRect(x, y + 6, 70, 14, 4, body);
    r.polygon({ { x + 14, y + 20 }, { x + 54, y + 20 }, { x + 46, y + 32 }, { x + 22, y + 32 } }, body);
    r.polygon({ { x + 20, y + 21 }, { x + 33, y + 21 }, { x + 33, y + 29 }, { x + 24, y + 29 } }, rgb(40, 50, 60));
    r.polygon({ { x + 36, y + 21 }, { x + 49, y + 21 }, { x + 44, y + 29 }, { x + 36, y + 29 } }, rgb(40, 50, 60));
    r.line({ x + 22, y + 28 }, { x + 30, y + 22 }, 1, rgb(160, 170, 180));
    r.circle(x + 16, y + 6, 7, rgb(26, 26, 28), 16);
    r.circle(x + 54, y + 6, 7, rgb(26, 26, 28), 16);
    r.circle(x + 16, y + 6, 3, rgb(90, 90, 96), 10);
    r.circle(x + 54, y + 6, 3, rgb(90, 90, 96), 10);
    r.circle(x + 40, y + 13, 3, rust, 10);
    r.circle(x + 8, y + 15, 2, rust, 8);
}

static void fireBarrel(Renderer2D& r, float x, float y, float time)
{
    r.roundedRect(x - 10, y, 20, 26, 3, rgb(70, 82, 74));
    r.rect(x - 10, y + 6, 20, 2, rgb(46, 54, 48));
    r.rect(x - 10, y + 18, 20, 2, rgb(46, 54, 48));

    float f = std::sin(time * 9 + x) * 2;
    r.circle(x, y + 32, 16, rgb(255, 150, 40, 40), 18);
    r.triangle({ x - 9, y + 26 }, { x + 9, y + 26 }, { x + f, y + 44 }, rgb(240, 110, 30));
    r.triangle({ x - 5, y + 26 }, { x + 6, y + 26 }, { x - f, y + 38 }, rgb(255, 210, 80));
}

static void streetLamp(Renderer2D& r, float x, float y, float time)
{
    r.rect(x - 2, y, 4, 38, rgb(50, 52, 58));
    r.rect(x - 2, y + 36, 14, 3, rgb(50, 52, 58));
    r.rect(x + 7, y + 32, 9, 4, rgb(40, 42, 46));

    bool on = std::sin(time * 13) + std::sin(time * 3.1f) > -1.2f;
    if (on)
    {
        r.circle(x + 11, y + 31, 10, rgb(255, 235, 150, 50), 16);
        r.circle(x + 11, y + 31, 3, rgb(255, 240, 170), 10);
        r.ellipse(x + 11, y, 22, 5, rgb(255, 235, 150, 35), 16);
    }
}

static void sandbags(Renderer2D& r, float x, float y)
{
    Color bag = rgb(150, 132, 92);
    Color dark = rgb(112, 96, 64);
    for (int i = 0; i < 4; i++)
    {
        r.ellipse(x + i * 15, y + 5, 9, 6, dark, 14);
        r.ellipse(x + i * 15, y + 6, 8, 5, bag, 14);
    }
    for (int i = 0; i < 3; i++)
    {
        r.ellipse(x + 7 + i * 15, y + 15, 9, 6, dark, 14);
        r.ellipse(x + 7 + i * 15, y + 16, 8, 5, bag, 14);
    }
}

static void brokenFence(Renderer2D& r, float x, float y)
{
    Color wood = rgb(96, 72, 50);
    int heights[5] = { 26, 20, 28, 12, 24 };
    for (int i = 0; i < 5; i++)
        r.rect(x + i * 11, y, 6, (float)heights[i], wood);
    r.rect(x - 2, y + 8, 50, 3, rgb(76, 56, 38));
    r.line({ x - 2, y + 18 }, { x + 30, y + 22 }, 3, rgb(76, 56, 38));
}

static void drawSky(Renderer2D& r, float time)
{
    r.rectGradient(0, FIELD_TOP, (float)VIRTUAL_W, VIRTUAL_H - FIELD_TOP, rgb(46, 34, 62), rgb(14, 12, 28));

    for (int i = 0; i < 40; i++)
    {
        float x = std::fmod(i * 197.0f, (float)VIRTUAL_W);
        float y = FIELD_TOP + 30 + std::fmod(i * 53.0f, 90.0f);
        float twinkle = 0.5f + 0.5f * std::sin(time * 2 + i);
        r.circle(x, y, 1.2f, Color(1, 1, 1, 0.3f + 0.5f * twinkle), 6);
    }

    r.circle(1100, 670, 30, rgb(255, 250, 220, 40), 32);
    r.circle(1100, 670, 22, rgb(236, 232, 210), 32);
    r.circle(1093, 676, 5, rgb(210, 206, 186), 12);
    r.circle(1108, 664, 3, rgb(210, 206, 186), 10);

    for (int i = 0; i < 3; i++)
    {
        float x = std::fmod(i * 450 + time * 12, (float)VIRTUAL_W + 300) - 150;
        r.ellipse(x, 690 - i * 18, 90, 10, rgb(70, 60, 90, 90), 20);
    }

    for (int i = 0; i < 16; i++)
    {
        float w = 50 + std::fmod(i * 37.0f, 40.0f);
        float h = 20 + std::fmod(i * 23.0f, 40.0f);
        r.rect(i * 82.0f, FIELD_TOP, w, h, rgb(22, 18, 30));
    }
}

static void drawLaneGround(Renderer2D& r, int lane)
{
    float y = laneBottomY(lane);
    float w = (float)VIRTUAL_W;

    Color back = lane % 2 == 0 ? rgb(56, 72, 46) : rgb(50, 66, 42);
    Color front = lane % 2 == 0 ? rgb(78, 92, 56) : rgb(72, 86, 52);
    r.rectGradient(0, y, w, LANE_HEIGHT, front, back);

    r.rectGradient(0, y, w, 44, rgb(104, 82, 58), rgb(92, 74, 52));
    r.rect(0, y + 44, w, 2, rgb(70, 60, 40));

    for (int i = 0; i < 22; i++)
    {
        float x = HOME_BASE_RIGHT + std::fmod(i * 173.0f + lane * 97.0f, w - HOME_BASE_RIGHT);
        if (i % 2 == 0)
        {
            r.ellipse(x, y + 10 + (i % 3) * 9, 4, 2.5f, rgb(76, 62, 46), 8);
        }
        else
        {
            float gy = y + 50 + (i % 4) * 12;
            r.line({ x, gy }, { x - 3, gy + 7 }, 1.5f, rgb(96, 116, 64));
            r.line({ x, gy }, { x + 3, gy + 8 }, 1.5f, rgb(96, 116, 64));
        }
    }

    r.rect(0, y, w, 3, rgb(30, 34, 22));
}

static void drawProps(Renderer2D& r, float time)
{
    deadTree(r, 470, propBase(0));
    grave(r, 820, propBase(0));
    grave(r, 850, propBase(0) + 2);

    wreckedCar(r, 640, propBase(1));
    fireBarrel(r, 1020, propBase(1), time);

    streetLamp(r, 380, propBase(2), time);
    brokenFence(r, 900, propBase(2));

    sandbags(r, 560, propBase(3));
    fireBarrel(r, 760, propBase(3), time);
    deadTree(r, 1110, propBase(3));
}

namespace Environment
{
    void drawBackground(Renderer2D& r, float time)
    {
        drawSky(r, time);

        r.rect(0, 0, (float)VIRTUAL_W, FIELD_BOTTOM, rgb(40, 32, 26));

        for (int lane = 0; lane < NUM_LANES; lane++)
            drawLaneGround(r, lane);

        drawProps(r, time);
    }

    void drawHomeBase(Renderer2D& r, float time)
    {
        float h = FIELD_TOP - FIELD_BOTTOM;
        r.rect(0, FIELD_BOTTOM, 124, h, rgb(92, 62, 40));

        for (int i = 0; i * 18 < h; i++)
        {
            Color plank = i % 2 == 0 ? rgb(104, 72, 46) : rgb(96, 66, 42);
            r.rect(0, FIELD_BOTTOM + i * 18, 124, 16, plank);
        }

        r.rect(120, FIELD_BOTTOM, 10, h, rgb(60, 40, 26));

        for (int lane = 0; lane < NUM_LANES; lane++)
        {
            float g = laneGroundY(lane);

            float flicker = 0.75f + 0.25f * std::sin(time * 7 + lane * 2.3f);
            r.rect(26, g + 46, 56, 50, rgb(40, 26, 18));
            r.rect(30, g + 50, 48, 42, Color(1.0f, 0.72f, 0.3f, flicker));
            r.circle(60, g + 64, 7, rgb(60, 40, 30), 14);
            r.rect(53, g + 50, 14, 10, rgb(60, 40, 30));
            r.rect(52, g + 50, 4, 42, rgb(40, 26, 18));
            r.rect(30, g + 69, 48, 4, rgb(40, 26, 18));
            r.line({ 24, g + 48 }, { 84, g + 94 }, 5, rgb(120, 86, 52));

            for (int i = 0; i < 3; i++)
            {
                float sy = g + 6 + i * 16;
                r.triangle({ 128, sy }, { 128, sy + 8 }, { 160, sy + 4 }, rgb(150, 116, 74));
                r.line({ 130, sy + 4 }, { 158, sy + 4 }, 1, rgb(100, 76, 46));
            }

            r.ellipse(104, g + 2, 14, 7, rgb(112, 96, 64), 14);
            r.ellipse(104, g + 3, 12, 6, rgb(150, 132, 92), 14);
            r.ellipse(104, g + 13, 12, 6, rgb(150, 132, 92), 14);

            r.rect(0, laneBottomY(lane), 130, 6, rgb(58, 38, 24));
        }

        r.roundedRect(8, FIELD_TOP - 30, 112, 24, 4, rgb(60, 40, 26));
        r.roundedRect(11, FIELD_TOP - 27, 106, 18, 3, rgb(176, 150, 104));
        Text::draw(r, "SAFE HOUSE", 64, FIELD_TOP - 23, 10, rgb(110, 30, 24), Align::Center);
    }

    void drawForeground(Renderer2D& r, float time)
    {
        for (int lane = 0; lane < NUM_LANES; lane++)
        {
            for (int i = 0; i < 4; i++)
            {
                float x = std::fmod(i * 380.0f + lane * 140 + time * (14 + lane * 3), (float)VIRTUAL_W + 400) - 200;
                r.ellipse(x, laneBottomY(lane) + 12, 170, 14, Color(0.8f, 0.85f, 0.9f, 0.06f), 24);
            }
        }
    }
}

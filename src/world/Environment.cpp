// ============================================================================
//  PLACEHOLDER - Member 4 replaces this file.
//  TODO (Member 4): themed background, home base model (must stand out),
//  at least 5 environment props that fit the zombie setting.
// ============================================================================
#include "Environment.h"
#include "../Config.h"
#include "../core/Renderer2D.h"
#include "../core/Text.h"

namespace Environment
{
    void drawBackground(Renderer2D& r, float)
    {
        r.rectGradient(0, 0, (float)VIRTUAL_W, (float)VIRTUAL_H, Colors::rgb(30, 25, 40), Colors::rgb(70, 50, 70));
        r.circle(1100, 560, 40, Colors::rgb(230, 225, 200));
        for (int lane = 0; lane < NUM_LANES; ++lane)
        {
            Color c = (lane % 2 == 0) ? Colors::rgb(70, 80, 50) : Colors::rgb(60, 70, 45);
            r.rect(0, laneBottomY(lane), (float)VIRTUAL_W, LANE_HEIGHT, c);
            r.rect(0, laneBottomY(lane), (float)VIRTUAL_W, 2, Colors::rgb(30, 35, 20));
        }
    }

    void drawHomeBase(Renderer2D& r, float)
    {
        r.rect(0, FIELD_BOTTOM, HOME_BASE_RIGHT, FIELD_TOP - FIELD_BOTTOM, Colors::rgb(110, 80, 50));
        r.rectOutline(0, FIELD_BOTTOM, HOME_BASE_RIGHT, FIELD_TOP - FIELD_BOTTOM, 4, Colors::rgb(40, 25, 15));
        Text::draw(r, "BASE", HOME_BASE_RIGHT * 0.5f, 300, 20, Colors::White, Align::Center);
    }

    void drawForeground(Renderer2D&, float) {}
}

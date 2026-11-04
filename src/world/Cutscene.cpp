// ============================================================================
//  PLACEHOLDER - Member 4 replaces this file with the real 10-20 s intro.
// ============================================================================
#include "Cutscene.h"
#include "../Config.h"
#include "../core/Renderer2D.h"
#include "../core/Text.h"

namespace
{
    float g_time = 0.0f;
    const float kDuration = 4.0f;
}

namespace Cutscene
{
    void  reset()          { g_time = 0.0f; }
    void  update(float dt) { g_time += dt; }
    bool  finished()       { return g_time >= kDuration; }
    float duration()       { return kDuration; }

    void draw(Renderer2D& r)
    {
        r.fullscreen(Colors::rgb(10, 8, 12));
        float zombieX = 1300.0f - g_time * 120.0f;
        r.rect(zombieX, 200, 40, 90, Colors::rgb(110, 150, 80));
        r.rect(300, 200, 40, 90, Colors::rgb(60, 110, 200));
        Text::drawShadowed(r, "CUTSCENE PLACEHOLDER (MEMBER 4)", VIRTUAL_W * 0.5f, 520, 24,
                           Colors::White, Align::Center);
    }
}

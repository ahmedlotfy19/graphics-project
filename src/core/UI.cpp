#include "UI.h"
#include "Input.h"
#include "Text.h"
#include "Audio.h"

#include <algorithm>
#include <cmath>

namespace UI
{
    bool button(Renderer2D& r, const std::string& label, float x, float y, float w, float h, bool enabled)
    {
        bool hover = enabled && Input::mouseInRect(x, y, w, h);
        bool pressed = hover && Input::mouseDown(GLFW_MOUSE_BUTTON_LEFT);

        Color base   = Colors::rgb(58, 38, 34);
        Color top    = hover ? Colors::rgb(150, 44, 36) : Colors::rgb(104, 30, 28);
        Color border = hover ? Colors::rgb(240, 200, 120) : Colors::rgb(150, 110, 70);
        if (!enabled) { top = Colors::rgb(60, 60, 60); border = Colors::rgb(90, 90, 90); }

        float sink = pressed ? 2.0f : 0.0f;
        r.roundedRect(x - 3, y - 3 - sink, w + 6, h + 6, 10, border);
        r.roundedRect(x, y - sink, w, h, 8, base);
        r.rectGradient(x + 4, y + 4 - sink, w - 8, h - 8, base, top);
        r.rect(x + 8, y + h - 10 - sink, w - 16, 3, Color(1, 1, 1, hover ? 0.25f : 0.12f));

        float size = std::min(24.0f, h * 0.42f);
        Text::drawShadowed(r, label, x + w * 0.5f, y + h * 0.5f - size * 0.5f - sink, size,
                           enabled ? Colors::rgb(250, 236, 210) : Colors::rgb(140, 140, 140),
                           Align::Center);

        if (hover && Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            Input::consumeClick();
            Audio::play(Sfx::Click);
            return true;
        }
        return false;
    }

    void panel(Renderer2D& r, float x, float y, float w, float h)
    {
        r.roundedRect(x - 4, y - 4, w + 8, h + 8, 16, Colors::rgb(150, 110, 70));
        r.roundedRect(x, y, w, h, 13, Colors::rgb(24, 20, 22, 240));
        r.rectGradient(x + 10, y + 10, w - 20, h - 20, Colors::rgb(30, 26, 28, 240), Colors::rgb(48, 36, 36, 240));
    }

    void healthBar(Renderer2D& r, float cx, float y, int hp, int maxHp, float width)
    {
        if (maxHp <= 0) return;
        hp = std::clamp(hp, 0, maxHp);
        float frac = (float)hp / maxHp;
        const float h = 9.0f;
        float x = cx - width * 0.5f;

        Color fill = frac > 0.6f ? Colors::rgb(70, 200, 80)
                   : frac > 0.3f ? Colors::rgb(235, 190, 50)
                                 : Colors::rgb(220, 50, 40);

        r.rect(x - 2, y - 2, width + 4, h + 4, Colors::rgb(10, 10, 10, 220));
        r.rect(x, y, width, h, Colors::rgb(60, 20, 20, 230));
        r.rect(x, y, width * frac, h, fill);
        r.rect(x, y + h - 3, width * frac, 2, Color(1, 1, 1, 0.3f));

        float seg = width / maxHp;
        for (int i = 1; i < maxHp; ++i)
            r.rect(x + seg * i - 1, y, 2, h, Colors::rgb(10, 10, 10, 220));
    }

    void coinIcon(Renderer2D& r, float cx, float cy, float radius, float spin)
    {
        float sx = spin == 0.0f ? 1.0f : std::max(0.15f, std::fabs(std::cos(spin)));
        r.push();
        r.translate(cx, cy);
        r.scale(sx, 1.0f);
        r.circle(0, -2, radius, Colors::rgb(150, 95, 20));
        r.circle(0, 0, radius, Colors::rgb(240, 180, 40));
        r.circle(0, 0, radius * 0.75f, Colors::rgb(255, 210, 70));
        r.ring(0, 0, radius * 0.62f, radius * 0.75f, Colors::rgb(200, 140, 30));
        r.rect(-radius * 0.12f, -radius * 0.42f, radius * 0.24f, radius * 0.84f, Colors::rgb(200, 140, 30));
        r.circle(-radius * 0.35f, radius * 0.35f, radius * 0.16f, Color(1, 1, 1, 0.6f));
        r.pop();
    }
}

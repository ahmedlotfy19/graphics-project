#include "UI.h"
#include "Input.h"
#include "Text.h"
#include "Audio.h"

#include <cmath>

namespace UI
{
    bool button(Renderer2D& r, const std::string& label, float x, float y, float w, float h)
    {
        bool hover = Input::mouseInRect(x, y, w, h);

        Color border = hover ? Colors::rgb(240, 200, 120) : Colors::rgb(150, 110, 70);
        Color top    = hover ? Colors::rgb(150, 44, 36) : Colors::rgb(104, 30, 28);
        Color bottom = Colors::rgb(58, 38, 34);

        r.roundedRect(x - 3, y - 3, w + 6, h + 6, 10, border);
        r.roundedRect(x, y, w, h, 8, bottom);
        r.rectGradient(x + 4, y + 4, w - 8, h - 8, bottom, top);

        float size = h * 0.42f;
        if (size > 24) size = 24;
        Text::drawShadowed(r, label, x + w / 2, y + h / 2 - size / 2, size, Colors::rgb(250, 236, 210), Align::Center);

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
        r.roundedRect(x, y, w, h, 13, Colors::rgb(30, 25, 27, 240));
    }

    void healthBar(Renderer2D& r, float cx, float y, int hp, int maxHp)
    {
        if (hp < 0) hp = 0;
        float width = 56;
        float height = 9;
        float x = cx - width / 2;
        float percent = (float)hp / maxHp;

        Color fill = Colors::rgb(70, 200, 80);
        if (percent <= 0.6f) fill = Colors::rgb(235, 190, 50);
        if (percent <= 0.3f) fill = Colors::rgb(220, 50, 40);

        r.rect(x - 2, y - 2, width + 4, height + 4, Colors::rgb(10, 10, 10, 220));
        r.rect(x, y, width, height, Colors::rgb(60, 20, 20, 230));
        r.rect(x, y, width * percent, height, fill);

        float segment = width / maxHp;
        for (int i = 1; i < maxHp; i++)
            r.rect(x + segment * i - 1, y, 2, height, Colors::rgb(10, 10, 10, 220));
    }

    void coinIcon(Renderer2D& r, float cx, float cy, float radius, float spin)
    {
        float squash = std::fabs(std::cos(spin));
        if (squash < 0.15f) squash = 0.15f;

        r.push();
        r.translate(cx, cy);
        r.scale(squash, 1);
        r.circle(0, -2, radius, Colors::rgb(150, 95, 20));
        r.circle(0, 0, radius, Colors::rgb(240, 180, 40));
        r.circle(0, 0, radius * 0.75f, Colors::rgb(255, 210, 70));
        r.ring(0, 0, radius * 0.62f, radius * 0.75f, Colors::rgb(200, 140, 30));
        r.rect(-radius * 0.12f, -radius * 0.42f, radius * 0.24f, radius * 0.84f, Colors::rgb(200, 140, 30));
        r.pop();
    }
}

#pragma once
#include "Renderer2D.h"
#include <string>

namespace UI
{
    bool button(Renderer2D& r, const std::string& label, float x, float y, float w, float h);
    void panel(Renderer2D& r, float x, float y, float w, float h);
    void healthBar(Renderer2D& r, float cx, float y, int hp, int maxHp);
    void coinIcon(Renderer2D& r, float cx, float cy, float radius, float spin = 0.0f);
}

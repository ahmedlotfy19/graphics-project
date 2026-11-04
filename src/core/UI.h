#pragma once
// ============================================================================
//  UI  -  small immediate-mode widgets.  Owner: Member 1 (Core).
//
//      if (UI::button(r, "START", 540, 300, 200, 56)) startGame();
//      UI::healthBar(r, enemy.x, top + 10, enemy.hp, enemy.maxHp);
// ============================================================================
#include "Renderer2D.h"
#include <string>

namespace UI
{
    // Draws a button; returns true on the frame it is clicked. (x,y) = bottom-left.
    bool button(Renderer2D& r, const std::string& label, float x, float y, float w, float h,
                bool enabled = true);

    // Dark rounded panel with a border, used behind menus.
    void panel(Renderer2D& r, float x, float y, float w, float h);

    // Segmented health bar centred on cx, bottom edge at y.
    // One segment per hit point, colour goes green -> yellow -> red as health drops.
    void healthBar(Renderer2D& r, float cx, float y, int hp, int maxHp, float width = 56.0f);

    // The collectible coin icon (also used by the HUD counter).
    void coinIcon(Renderer2D& r, float cx, float cy, float radius, float spin = 0.0f);
}

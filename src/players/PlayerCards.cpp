// ============================================================================
//  PlayerCards  -  Member 2
//  Cards show a mini version of each real model. Locked cards (not enough coins)
//  are grayscale with a padlock and ignore the mouse; unlocked cards lift on hover
//  and preview the attack animation.
// ============================================================================
#include "PlayerCards.h"
#include "../game/World.h"
#include "../core/Renderer2D.h"
#include "../core/Text.h"
#include "../core/UI.h"
#include "../core/Input.h"
#include "../core/Audio.h"

#include <GLFW/glfw3.h>
#include <cmath>
#include <string>

namespace
{
    const char* kNames[] = { "MELEE", "RANGED", "DEFENSE" };

    float cardX(int i) { return PlayerCards::CARDS_X + i * (PlayerCards::CARD_W + PlayerCards::CARD_GAP); }

    // horizontal nudge so each model looks centred on its card (the crossbow and shield stick out)
    float centreOffset(PlayerType t)
    {
        switch (t)
        {
        case PlayerType::Ranged:  return -9.0f;
        case PlayerType::Defense: return -6.0f;
        default:                  return 0.0f;
        }
    }

    void padlock(Renderer2D& r, float cx, float cy)
    {
        r.ring(cx, cy + 6, 4.2f, 6.4f, Color(0.95f, 0.95f, 0.95f, 1.0f), 14);
        r.roundedRect(cx - 8, cy - 6, 16, 13, 2, Color(1.0f, 1.0f, 1.0f, 1.0f));
        r.circle(cx, cy, 2.0f, Color(0.2f, 0.2f, 0.2f, 1.0f), 8);
        r.rect(cx - 0.8f, cy - 4.5f, 1.6f, 4.0f, Color(0.2f, 0.2f, 0.2f, 1.0f));
    }
}

namespace PlayerCards
{
    int cost(PlayerType t)
    {
        switch (t)
        {
        case PlayerType::Ranged:  return RANGED_COST;
        case PlayerType::Defense: return DEFENSE_COST;
        default:                  return MELEE_COST;
        }
    }

    // Re-evaluated every frame, so cards unlock and re-lock automatically with the coin count.
    bool isUnlocked(const World& w, PlayerType t) { return w.coins >= cost(t); }

    void update(World& w)
    {
        w.cards.hovered = -1;
        for (int i = 0; i < (int)PlayerType::Count; ++i)
        {
            PlayerType t = (PlayerType)i;
            if (!isUnlocked(w, t)) continue;                               // locked: no hover, no click
            if (!Input::mouseInRect(cardX(i), CARDS_Y, CARD_W, CARD_H)) continue;

            w.cards.hovered = i;
            if (Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                Input::consumeClick();                                     // don't toggle the lane below
                bool any = false;
                for (int lane = 0; lane < NUM_LANES; ++lane)
                {
                    if (!w.lanes.selected[lane]) continue;
                    any = true;
                    if (w.coins < cost(t)) continue;
                    w.coins -= cost(t);
                    Players::spawn(w, t, lane);
                }
                if (!any) w.cards.warnUntil = w.time + 1.6f;               // tell the player what to do
            }
        }
    }

    void draw(Renderer2D& r, const World& w)
    {
        for (int i = 0; i < (int)PlayerType::Count; ++i)
        {
            PlayerType t = (PlayerType)i;
            bool unlocked = isUnlocked(w, t);
            bool hover = w.cards.hovered == i;
            float x = cardX(i), y = CARDS_Y + (hover ? 4.0f : 0.0f);

            r.push();
            r.setGrayscale(!unlocked);                                     // locked = grayscale
            r.roundedRect(x - 3, y - 3, CARD_W + 6, CARD_H + 6, 10,
                          hover ? Colors::rgb(255, 220, 120) : Colors::rgb(150, 110, 70));
            r.roundedRect(x, y, CARD_W, CARD_H, 8, Colors::rgb(230, 210, 170));
            r.rectGradient(x + 4, y + 30, CARD_W - 8, CARD_H - 36, Colors::rgb(190, 170, 130), Colors::rgb(240, 226, 190));

            // mini model (hovering previews the attack)
            r.push();
            r.translate(x + CARD_W * 0.5f + centreOffset(t) * 0.58f, y + 33.0f);
            r.scale(0.58f);
            float preview = hover ? std::fmod(w.time * 1.1f, 1.0f) : -1.0f;
            Players::drawModel(r, t, 0.0f, 0.0f, w.time, Pose::Normal, 0.0f, preview, false);
            r.pop();

            Text::draw(r, kNames[i], x + CARD_W * 0.5f, y + 19, 10, Colors::rgb(40, 30, 20), Align::Center);
            UI::coinIcon(r, x + CARD_W * 0.5f - 14.0f, y + 10.0f, 5.5f);
            Text::draw(r, std::to_string(cost(t)), x + CARD_W * 0.5f + 6.0f, y + 4, 11, Colors::rgb(140, 90, 10), Align::Center);

            if (!unlocked) padlock(r, x + CARD_W - 15.0f, y + CARD_H - 17.0f);
            r.pop();
        }

        // hint when a card was clicked with no lane selected
        float left = w.cards.warnUntil - w.time;
        if (left > 0.0f)
        {
            float a = std::min(1.0f, left / 0.4f);
            r.push();
            r.setAlpha(a);
            Text::drawShadowed(r, "SELECT A LANE FIRST", CARDS_X + 150.0f, 566.0f, 16, Colors::rgb(255, 120, 90), Align::Center);
            r.pop();
        }
    }
}

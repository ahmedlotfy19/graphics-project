// ============================================================================
//  PLACEHOLDER - Member 2 replaces the visuals (draw mini models on cards).
//  Current flow: select lane(s) first, then click an unlocked card ->
//  one player of that type is placed in each selected lane you can afford.
//  Change the flow if your team prefers, but keep the lock/unlock rules.
// ============================================================================
#include "PlayerCards.h"
#include "../game/World.h"
#include "../core/Renderer2D.h"
#include "../core/Text.h"
#include "../core/Input.h"
#include "../core/Audio.h"

#include <string>

namespace
{
    const char* kNames[] = { "MELEE", "RANGED", "DEFENSE" };

    float cardX(int i) { return PlayerCards::CARDS_X + i * (PlayerCards::CARD_W + PlayerCards::CARD_GAP); }
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

    bool isUnlocked(const World& w, PlayerType t) { return w.coins >= cost(t); }

    void update(World& w)
    {
        w.cards.hovered = -1;
        for (int i = 0; i < (int)PlayerType::Count; ++i)
        {
            PlayerType t = (PlayerType)i;
            if (!isUnlocked(w, t)) continue;                       // locked: no hover, no click
            if (!Input::mouseInRect(cardX(i), CARDS_Y, CARD_W, CARD_H)) continue;

            w.cards.hovered = i;
            if (Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                Input::consumeClick();
                for (int lane = 0; lane < NUM_LANES; ++lane)
                {
                    if (!w.lanes.selected[lane] || w.coins < cost(t)) continue;
                    w.coins -= cost(t);
                    Players::spawn(w, t, lane);
                }
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
            r.setGrayscale(!unlocked);
            r.roundedRect(x - 3, y - 3, CARD_W + 6, CARD_H + 6, 10,
                          hover ? Colors::rgb(255, 220, 120) : Colors::rgb(150, 110, 70));
            r.roundedRect(x, y, CARD_W, CARD_H, 8, Colors::rgb(230, 210, 170));
            Color c = i == 0 ? Colors::rgb(60, 110, 200) : i == 1 ? Colors::rgb(60, 170, 90) : Colors::rgb(150, 150, 160);
            r.rect(x + 26, y + 34, 40, 50, c);   // TODO Member 2: mini model
            Text::draw(r, kNames[i], x + CARD_W * 0.5f, y + 20, 10, Colors::rgb(40, 30, 20), Align::Center);
            Text::draw(r, std::to_string(cost(t)), x + CARD_W * 0.5f, y + 5, 11, Colors::rgb(140, 90, 10), Align::Center);
            r.pop();
        }
    }
}

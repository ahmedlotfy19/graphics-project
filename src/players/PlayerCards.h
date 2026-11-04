#pragma once
// ============================================================================
//  PlayerCards  -  the shop cards in the HUD.  Owner: MEMBER 2
//  Placeholder: simple cards. Replace with mini versions of the real models.
//
//  Rules (spec 4.2):
//   * card is unlocked while  world.coins >= cost  (re-locks if coins drop below)
//   * locked cards are grayscale and ignore hover/clicks
//   * Use Input::consumeClick() when a card is clicked so Lanes don't react too.
// ============================================================================
#include "Players.h"

class Renderer2D;
struct World;

struct CardState
{
    int hovered = -1;
    int selected = -1;
};

namespace PlayerCards
{
    int  cost(PlayerType t);
    bool isUnlocked(const World& w, PlayerType t);

    constexpr float CARDS_X  = 200.0f;
    constexpr float CARDS_Y  = 612.0f;
    constexpr float CARD_W   = 92.0f;
    constexpr float CARD_H   = 100.0f;
    constexpr float CARD_GAP = 12.0f;

    void update(World& w);
    void draw(Renderer2D& r, const World& w);
}

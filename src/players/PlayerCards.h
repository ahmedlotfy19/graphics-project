#pragma once
// ============================================================================
//  PlayerCards  -  the shop cards in the HUD.  Owner: MEMBER 2
//
//  Rules (spec 4.2):
//   * a card is unlocked while  world.coins >= cost  (re-locks if coins drop below)
//   * locked cards are grayscale and ignore hover/clicks
//   * a card click calls Input::consumeClick() so the Lanes don't react to it too
//
//  Flow: select one or more lanes, then click an unlocked card -> one player of
//  that type is placed in each selected lane you can afford.
// ============================================================================
#include "Players.h"

class Renderer2D;
struct World;

struct CardState
{
    int hovered = -1;
    int selected = -1;
    float warnUntil = -1.0f;   // "select a lane first" message is shown until w.time reaches this
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

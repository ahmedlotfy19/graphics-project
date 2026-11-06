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

#include <cmath>
#include <string>

static const char* NAMES[] = { "MELEE", "RANGED", "DEFENSE" };

static float cardX(int i)
{
    return PlayerCards::CARDS_X + i * (PlayerCards::CARD_W + PlayerCards::CARD_GAP);
}

static float modelOffset(PlayerType t)
{
    if (t == PlayerType::Ranged)  return -9;
    if (t == PlayerType::Defense) return -6;
    return 0;
}

static void padlock(Renderer2D& r, float cx, float cy)
{
    r.ring(cx, cy + 6, 4.2f, 6.4f, Color(0.95f, 0.95f, 0.95f, 1), 14);
    r.roundedRect(cx - 8, cy - 6, 16, 13, 2, Colors::White);
    r.circle(cx, cy, 2, Color(0.2f, 0.2f, 0.2f, 1), 8);
    r.rect(cx - 0.8f, cy - 4.5f, 1.6f, 4, Color(0.2f, 0.2f, 0.2f, 1));
}

static bool anyLaneSelected(const World& w)
{
    for (int lane = 0; lane < NUM_LANES; lane++)
        if (w.lanes.selected[lane]) return true;
    return false;
}

static float defenseX(float mouseX)
{
    float left = HOME_BASE_RIGHT + 30;
    float right = FIELD_RIGHT - 40;
    if (mouseX < left) return left;
    if (mouseX > right) return right;
    return mouseX;
}

static void placePlayers(World& w, PlayerType t, float x)
{
    if (!anyLaneSelected(w))
    {
        w.cards.warnUntil = w.time + 1.6f;
        return;
    }

    for (int lane = 0; lane < NUM_LANES; lane++)
    {
        if (w.lanes.selected[lane] && w.coins >= PlayerCards::cost(t))
        {
            w.coins -= PlayerCards::cost(t);
            Players::spawn(w, t, lane, x);
        }
    }
}

static void clickCard(World& w, PlayerType t)
{
    if (t != PlayerType::Defense)
    {
        w.cards.placingDefense = false;
        placePlayers(w, t, PLAYER_SPAWN_X);
    }
    else if (!anyLaneSelected(w))
    {
        w.cards.warnUntil = w.time + 1.6f;
    }
    else
    {
        w.cards.placingDefense = !w.cards.placingDefense;
    }
}

namespace PlayerCards
{
    int cost(PlayerType t)
    {
        if (t == PlayerType::Ranged)  return RANGED_COST;
        if (t == PlayerType::Defense) return DEFENSE_COST;
        return MELEE_COST;
    }

    bool isUnlocked(const World& w, PlayerType t)
    {
        return w.coins >= cost(t);
    }

    void update(World& w)
    {
        w.cards.hovered = -1;

        for (int i = 0; i < 3; i++)
        {
            PlayerType t = (PlayerType)i;
            if (!isUnlocked(w, t)) continue;
            if (!Input::mouseInRect(cardX(i), CARDS_Y, CARD_W, CARD_H)) continue;

            w.cards.hovered = i;
            if (Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT))
            {
                Input::consumeClick();
                clickCard(w, t);
            }
        }

        glm::vec2 mouse = Input::mouse();
        bool onField = laneAtY(mouse.y) >= 0;
        if (w.cards.placingDefense && onField && Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            Input::consumeClick();
            placePlayers(w, PlayerType::Defense, defenseX(mouse.x));
            w.cards.placingDefense = false;
        }
    }

    void draw(Renderer2D& r, const World& w)
    {
        for (int i = 0; i < 3; i++)
        {
            PlayerType t = (PlayerType)i;
            bool unlocked = isUnlocked(w, t);
            bool placing = w.cards.placingDefense && t == PlayerType::Defense;
            bool hover = w.cards.hovered == i || placing;
            float x = cardX(i);
            float y = hover ? CARDS_Y + 4 : CARDS_Y;
            float middle = x + CARD_W / 2;

            r.push();
            r.setGrayscale(!unlocked);

            Color border = hover ? Colors::rgb(255, 220, 120) : Colors::rgb(150, 110, 70);
            r.roundedRect(x - 3, y - 3, CARD_W + 6, CARD_H + 6, 10, border);
            r.roundedRect(x, y, CARD_W, CARD_H, 8, Colors::rgb(230, 210, 170));
            r.rectGradient(x + 4, y + 30, CARD_W - 8, CARD_H - 36, Colors::rgb(190, 170, 130), Colors::rgb(240, 226, 190));

            float preview = hover ? fmod(w.time * 1.1f, 1.0f) : -1;
            r.push();
            r.translate(middle + modelOffset(t) * 0.58f, y + 33);
            r.scale(0.58f);
            Players::drawModel(r, t, 0, 0, w.time, Pose::Normal, 0, preview, false);
            r.pop();

            Text::draw(r, NAMES[i], middle, y + 19, 10, Colors::rgb(40, 30, 20), Align::Center);
            UI::coinIcon(r, middle - 14, y + 10, 5.5f);
            Text::draw(r, std::to_string(cost(t)), middle + 6, y + 4, 11, Colors::rgb(140, 90, 10), Align::Center);

            if (!unlocked)
                padlock(r, x + CARD_W - 15, y + CARD_H - 17);

            r.pop();
        }

        if (w.cards.placingDefense)
        {
            glm::vec2 mouse = Input::mouse();
            if (laneAtY(mouse.y) >= 0)
            {
                for (int lane = 0; lane < NUM_LANES; lane++)
                {
                    if (!w.lanes.selected[lane]) continue;
                    r.push();
                    r.setAlpha(0.45f);
                    Players::drawModel(r, PlayerType::Defense, defenseX(mouse.x), laneGroundY(lane), w.time,
                                       Pose::Normal, 0, -1, false);
                    r.pop();
                }
            }
            Text::drawShadowed(r, "CLICK ON THE FIELD TO PLACE THE DEFENSE", CARDS_X + 150, 566, 14,
                               Colors::rgb(255, 220, 120), Align::Center);
        }

        float timeLeft = w.cards.warnUntil - w.time;
        if (timeLeft > 0)
        {
            float alpha = timeLeft / 0.4f;
            if (alpha > 1) alpha = 1;

            r.push();
            r.setAlpha(alpha);
            Text::drawShadowed(r, "SELECT A LANE FIRST", CARDS_X + 150, 566, 16, Colors::rgb(255, 120, 90), Align::Center);
            r.pop();
        }
    }
}

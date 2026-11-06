#include "Lanes.h"
#include "../game/World.h"
#include "../core/Renderer2D.h"
#include "../core/Input.h"
#include "../core/Audio.h"

#include <cmath>

namespace Lanes
{
    void update(World& w)
    {
        glm::vec2 m = Input::mouse();
        w.lanes.hovered = (m.x >= 0 && m.x <= VIRTUAL_W) ? laneAtY(m.y) : -1;

        if (w.lanes.hovered >= 0 && Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            bool& sel = w.lanes.selected[w.lanes.hovered];
            sel = !sel;
            Audio::play(Sfx::LaneSelect);
        }
    }

    void draw(Renderer2D& r, const World& w)
    {
        float pulse = 0.5f + 0.5f * std::sin(w.time * 5);

        for (int lane = 0; lane < NUM_LANES; lane++)
        {
            float y = laneBottomY(lane);
            float mid = laneCenterY(lane);

            if (w.lanes.selected[lane])
            {
                r.rect(0, y, (float)VIRTUAL_W, LANE_HEIGHT, Colors::rgb(255, 220, 90, 40 + (int)(25 * pulse)));
                r.rectOutline(0, y, (float)VIRTUAL_W, LANE_HEIGHT, 4, Colors::rgb(255, 210, 70, 220));

                float x = HOME_BASE_RIGHT + 14 + 4 * pulse;
                r.triangle({ x, mid - 12 }, { x, mid + 12 }, { x + 16, mid }, Colors::rgb(255, 210, 70));
            }

            if (w.lanes.hovered == lane)
            {
                r.rect(0, y, (float)VIRTUAL_W, LANE_HEIGHT, Colors::rgb(255, 255, 255, 30));
                r.rectOutline(0, y, (float)VIRTUAL_W, LANE_HEIGHT, 2, Colors::rgb(255, 255, 255, 120));
            }
        }
    }
}

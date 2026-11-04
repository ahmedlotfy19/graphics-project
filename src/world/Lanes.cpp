// ============================================================================
//  PLACEHOLDER - Member 4 polishes (nicer highlight style, etc.).
//  Hover + click-toggle logic already follows the spec.
// ============================================================================
#include "Lanes.h"
#include "../game/World.h"
#include "../core/Renderer2D.h"
#include "../core/Input.h"
#include "../core/Audio.h"

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
        for (int lane = 0; lane < NUM_LANES; ++lane)
        {
            float y = laneBottomY(lane);
            if (w.lanes.selected[lane])
            {
                r.rect(0, y, (float)VIRTUAL_W, LANE_HEIGHT, Colors::rgb(255, 230, 120, 55));
                r.rectOutline(0, y, (float)VIRTUAL_W, LANE_HEIGHT, 3, Colors::rgb(255, 220, 90, 200));
            }
            else if (w.lanes.hovered == lane)
            {
                r.rect(0, y, (float)VIRTUAL_W, LANE_HEIGHT, Colors::rgb(255, 255, 255, 35));
            }
        }
    }
}

#pragma once
// ============================================================================
//  Input  -  keyboard + mouse state, with "pressed this frame" edge detection.
//  Owner: Member 1 (Core).
//
//      if (Input::keyPressed(GLFW_KEY_W) || Input::keyPressed(GLFW_KEY_UP)) ...
//      if (Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT)) ...
//      glm::vec2 m = Input::mouse();   // in VIRTUAL coordinates (1280x720, y up)
//
//  Clicks can be "consumed" so two systems don't react to the same click
//  (e.g. clicking a player card shouldn't also toggle a lane underneath):
//      if (Input::mousePressed(GLFW_MOUSE_BUTTON_LEFT) && overMyThing) {
//          Input::consumeClick(); ...
//      }
//  After consumeClick(), mousePressed(LEFT) returns false for the rest of the frame.
// ============================================================================
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE   // let glad provide the OpenGL headers (avoids include-order errors)
#endif
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Renderer2D;

namespace Input
{
    void init(GLFWwindow* window);
    void beginFrame();                        // call BEFORE glfwPollEvents()
    void update(const Renderer2D& r);         // call AFTER glfwPollEvents()

    bool keyDown(int key);                    // held
    bool keyPressed(int key);                 // went down this frame
    bool keyReleased(int key);                // went up this frame

    bool mouseDown(int button);
    bool mousePressed(int button);
    bool mouseReleased(int button);
    void consumeClick();

    glm::vec2 mouse();                        // virtual coords
    bool mouseInRect(float x, float y, float w, float h);
}

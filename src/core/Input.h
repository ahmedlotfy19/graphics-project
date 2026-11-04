#pragma once
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Renderer2D;

namespace Input
{
    void init(GLFWwindow* window);
    void beginFrame();
    void update(const Renderer2D& r);

    bool keyDown(int key);
    bool keyPressed(int key);
    bool keyReleased(int key);

    bool mouseDown(int button);
    bool mousePressed(int button);
    bool mouseReleased(int button);
    void consumeClick();

    glm::vec2 mouse();
    bool mouseInRect(float x, float y, float w, float h);
}

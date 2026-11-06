#pragma once
#define GLFW_INCLUDE_NONE
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
    bool mousePressed(int button);
    void consumeClick();

    glm::vec2 mouse();
    bool mouseInRect(float x, float y, float w, float h);
}

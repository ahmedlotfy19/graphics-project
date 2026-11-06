#include "Input.h"
#include "Renderer2D.h"

static GLFWwindow* window = nullptr;

static bool keysDown[GLFW_KEY_LAST + 1];
static bool keysPressed[GLFW_KEY_LAST + 1];
static bool buttonsDown[GLFW_MOUSE_BUTTON_LAST + 1];
static bool buttonsPressed[GLFW_MOUSE_BUTTON_LAST + 1];
static bool clickUsed = false;
static glm::vec2 mousePos(0, 0);

static void onKey(GLFWwindow*, int key, int, int action, int)
{
    if (key < 0 || key > GLFW_KEY_LAST) return;
    if (action == GLFW_PRESS)
    {
        keysDown[key] = true;
        keysPressed[key] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        keysDown[key] = false;
    }
}

static void onMouseButton(GLFWwindow*, int button, int action, int)
{
    if (button < 0 || button > GLFW_MOUSE_BUTTON_LAST) return;
    if (action == GLFW_PRESS)
    {
        buttonsDown[button] = true;
        buttonsPressed[button] = true;
    }
    else if (action == GLFW_RELEASE)
    {
        buttonsDown[button] = false;
    }
}

namespace Input
{
    void init(GLFWwindow* w)
    {
        window = w;
        glfwSetKeyCallback(window, onKey);
        glfwSetMouseButtonCallback(window, onMouseButton);
    }

    void beginFrame()
    {
        for (bool& k : keysPressed) k = false;
        for (bool& b : buttonsPressed) b = false;
        clickUsed = false;
    }

    void update(const Renderer2D& r)
    {
        double cx, cy;
        glfwGetCursorPos(window, &cx, &cy);

        int winW, winH, fbW, fbH;
        glfwGetWindowSize(window, &winW, &winH);
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (winW == 0 || winH == 0) return;

        float px = (float)cx * fbW / winW;
        float py = fbH - (float)cy * fbH / winH;
        mousePos.x = (px - r.viewportX()) / r.viewportScale();
        mousePos.y = (py - r.viewportY()) / r.viewportScale();
    }

    bool keyDown(int key)    { return keysDown[key]; }
    bool keyPressed(int key) { return keysPressed[key]; }
    bool mouseDown(int b)    { return buttonsDown[b]; }

    bool mousePressed(int b)
    {
        if (b == GLFW_MOUSE_BUTTON_LEFT && clickUsed) return false;
        return buttonsPressed[b];
    }

    void consumeClick() { clickUsed = true; }

    glm::vec2 mouse() { return mousePos; }

    bool mouseInRect(float x, float y, float w, float h)
    {
        return mousePos.x >= x && mousePos.x <= x + w && mousePos.y >= y && mousePos.y <= y + h;
    }
}

#include "Input.h"
#include "Renderer2D.h"
#include "../Config.h"

namespace
{
    const int kMaxKeys = GLFW_KEY_LAST + 1;
    const int kMaxButtons = GLFW_MOUSE_BUTTON_LAST + 1;

    bool g_keyDown[kMaxKeys] = {};
    bool g_keyPressed[kMaxKeys] = {};
    bool g_keyReleased[kMaxKeys] = {};
    bool g_btnDown[kMaxButtons] = {};
    bool g_btnPressed[kMaxButtons] = {};
    bool g_btnReleased[kMaxButtons] = {};
    bool g_clickConsumed = false;

    GLFWwindow* g_window = nullptr;
    glm::vec2 g_mouse(0.0f);

    void keyCallback(GLFWwindow*, int key, int, int action, int)
    {
        if (key < 0 || key >= kMaxKeys) return;
        if (action == GLFW_PRESS)        { g_keyDown[key] = true;  g_keyPressed[key] = true; }
        else if (action == GLFW_RELEASE) { g_keyDown[key] = false; g_keyReleased[key] = true; }
    }

    void mouseCallback(GLFWwindow*, int button, int action, int)
    {
        if (button < 0 || button >= kMaxButtons) return;
        if (action == GLFW_PRESS)        { g_btnDown[button] = true;  g_btnPressed[button] = true; }
        else if (action == GLFW_RELEASE) { g_btnDown[button] = false; g_btnReleased[button] = true; }
    }
}

namespace Input
{
    void init(GLFWwindow* window)
    {
        g_window = window;
        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseCallback);
    }

    void beginFrame()
    {
        for (int i = 0; i < kMaxKeys; ++i)    { g_keyPressed[i] = false; g_keyReleased[i] = false; }
        for (int i = 0; i < kMaxButtons; ++i) { g_btnPressed[i] = false; g_btnReleased[i] = false; }
        g_clickConsumed = false;
    }

    void update(const Renderer2D& r)
    {
        if (!g_window) return;
        double cx, cy;
        glfwGetCursorPos(g_window, &cx, &cy);
        int winW, winH, fbW, fbH;
        glfwGetWindowSize(g_window, &winW, &winH);
        glfwGetFramebufferSize(g_window, &fbW, &fbH);
        if (winW <= 0 || winH <= 0) return;   // minimised

        // window coords (y down) -> framebuffer pixels -> virtual (y up)
        float px = (float)cx * fbW / winW;
        float py = (float)cy * fbH / winH;
        float s = r.viewportScale();
        g_mouse.x = (px - r.viewportX()) / s;
        g_mouse.y = ((fbH - py) - r.viewportY()) / s;
    }

    bool keyDown(int key)     { return key >= 0 && key < kMaxKeys && g_keyDown[key]; }
    bool keyPressed(int key)  { return key >= 0 && key < kMaxKeys && g_keyPressed[key]; }
    bool keyReleased(int key) { return key >= 0 && key < kMaxKeys && g_keyReleased[key]; }

    bool mouseDown(int b)     { return b >= 0 && b < kMaxButtons && g_btnDown[b]; }
    bool mousePressed(int b)
    {
        if (b < 0 || b >= kMaxButtons) return false;
        if (b == GLFW_MOUSE_BUTTON_LEFT && g_clickConsumed) return false;
        return g_btnPressed[b];
    }
    bool mouseReleased(int b) { return b >= 0 && b < kMaxButtons && g_btnReleased[b]; }
    void consumeClick()       { g_clickConsumed = true; }

    glm::vec2 mouse() { return g_mouse; }

    bool mouseInRect(float x, float y, float w, float h)
    {
        return g_mouse.x >= x && g_mouse.x <= x + w && g_mouse.y >= y && g_mouse.y <= y + h;
    }
}

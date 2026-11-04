// ============================================================================
//  Entry point: window, OpenGL context, main loop.   Owner: Member 1
// ============================================================================
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <ctime>
#include <iostream>

#include "Config.h"
#include "core/Renderer2D.h"
#include "core/Input.h"
#include "core/Audio.h"
#include "game/Game.h"

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialise GLFW\n";
        return -1;
    }

    // Spec: modern OpenGL 3.3+
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);   // anti-aliased edges on shapes

    GLFWwindow* window = glfwCreateWindow(VIRTUAL_W, VIRTUAL_H, GAME_TITLE, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);   // vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialise GLAD\n";
        return -1;
    }
    glEnable(GL_MULTISAMPLE);

    std::srand((unsigned)std::time(nullptr));

    Renderer2D renderer;
    if (!renderer.init()) return -1;
    Input::init(window);
    Audio::init();   // failure is fine: the game runs silent

    Game game;
    game.init(window, &renderer);

    double last = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        float dt = (float)(now - last);
        last = now;
        if (dt > 0.05f) dt = 0.05f;   // avoid huge jumps after dragging the window, breakpoints, etc.

        Input::beginFrame();
        glfwPollEvents();

        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        if (fbW == 0 || fbH == 0) continue;   // minimised

        renderer.beginFrame(fbW, fbH);
        Input::update(renderer);
        game.frame(dt);
        renderer.endFrame();

        glfwSwapBuffers(window);
    }

    game.shutdown();
    Audio::shutdown();
    renderer.shutdown();
    glfwTerminate();
    return 0;
}

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
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(VIRTUAL_W, VIRTUAL_H, GAME_TITLE, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }
    glEnable(GL_MULTISAMPLE);

    srand((unsigned)time(nullptr));

    Renderer2D renderer;
    renderer.init();
    Input::init(window);
    Audio::init();

    Game game;
    game.init(window, &renderer);

    float lastFrame = (float)glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (deltaTime > 0.05f) deltaTime = 0.05f;

        Input::beginFrame();
        glfwPollEvents();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        if (width == 0 || height == 0) continue;

        renderer.beginFrame(width, height);
        Input::update(renderer);
        game.frame(deltaTime);
        renderer.endFrame();

        glfwSwapBuffers(window);
    }

    Audio::shutdown();
    renderer.shutdown();
    glfwTerminate();
    return 0;
}

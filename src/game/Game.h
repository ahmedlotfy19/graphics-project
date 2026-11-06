#pragma once
#include "World.h"

struct GLFWwindow;
class Renderer2D;

enum class GameState { MainMenu, SoundSettings, Cutscene, Playing, Paused, Won, Lost };

class Game
{
public:
    void init(GLFWwindow* window, Renderer2D* renderer);
    void frame(float dt);

private:
    void changeState(GameState s);
    void newGame(bool showCutscene);

    void updateGame(float dt);
    void removeDead();
    void checkWinLose();

    void drawGame();
    void drawHUD();

    void mainMenu();
    void soundMenu();
    void pauseMenu();
    void endMenu();
    void menuBackground();

    GLFWwindow* window = nullptr;
    Renderer2D* r = nullptr;
    World world;
    GameState state = GameState::MainMenu;
    float menuTime = 0;
};

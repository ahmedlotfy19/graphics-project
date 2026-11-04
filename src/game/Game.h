#pragma once
// ============================================================================
//  Game  -  state machine (menus / cutscene / gameplay), HUD, win/lose.
//  Owner: Member 1.
// ============================================================================
#include "World.h"

struct GLFWwindow;
class Renderer2D;

enum class GameState { MainMenu, SoundSettings, Cutscene, Playing, Paused, Won, Lost };

class Game
{
public:
    bool init(GLFWwindow* window, Renderer2D* renderer);
    void frame(float dt);          // update + draw one frame
    void shutdown();

private:
    void setState(GameState s);
    void startNewGame(bool withCutscene);

    void updatePlaying(float dt);
    void removeDead();
    void checkWinLose();

    void drawScene();              // world: background, characters, projectiles...
    void drawHealthBars();
    void drawHUD();
    void drawWaveBanner();

    void mainMenu();
    void soundSettings();
    void pauseMenu();
    void endMenu(bool won);
    void menuBackdrop();

    GLFWwindow* m_window = nullptr;
    Renderer2D* m_r = nullptr;
    World       m_world;
    GameState   m_state = GameState::MainMenu;
    float       m_stateTime = 0.0f;   // seconds since the last state change
    float       m_menuTime = 0.0f;    // animates menu backgrounds

    int   m_shownCoins = 0;           // for the coin counter "pop" animation
    float m_coinPulse = 0.0f;
    int   m_lastWave = 0;
    float m_bannerTime = 0.0f;
};

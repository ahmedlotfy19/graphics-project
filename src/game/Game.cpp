#include "Game.h"
#include "Collectibles.h"
#include "../core/Renderer2D.h"
#include "../core/Input.h"
#include "../core/Text.h"
#include "../core/UI.h"
#include "../core/Audio.h"
#include "../enemies/Combat.h"
#include "../world/Environment.h"
#include "../world/Cutscene.h"

#include <algorithm>
#include <string>

const float BUTTON_W = 260;
const float BUTTON_H = 56;
const float CENTER_X = VIRTUAL_W / 2.0f;

void Game::init(GLFWwindow* w, Renderer2D* renderer)
{
    window = w;
    r = renderer;
    changeState(GameState::MainMenu);
}

void Game::changeState(GameState s)
{
    state = s;

    if (s == GameState::MainMenu || s == GameState::SoundSettings || s == GameState::Cutscene)
        Audio::playMusic(Music::Menu);
    else if (s == GameState::Playing)
        Audio::playMusic(Music::Gameplay);
    else if (s == GameState::Won)
    {
        Audio::stopMusic();
        Audio::play(Sfx::Win);
    }
    else if (s == GameState::Lost)
    {
        Audio::stopMusic();
        Audio::play(Sfx::Lose);
    }
}

void Game::newGame(bool showCutscene)
{
    world.reset();
    if (showCutscene)
    {
        Cutscene::reset();
        changeState(GameState::Cutscene);
    }
    else
    {
        changeState(GameState::Playing);
    }
}

void Game::frame(float dt)
{
    menuTime += dt;

    switch (state)
    {
    case GameState::MainMenu:
        mainMenu();
        break;

    case GameState::SoundSettings:
        soundMenu();
        break;

    case GameState::Cutscene:
        Cutscene::update(dt);
        Cutscene::draw(*r);
        Text::drawShadowed(*r, "SPACE TO SKIP", VIRTUAL_W - 20.0f, 16, 14, Colors::White, Align::Right);
        if (Cutscene::finished() || Input::keyPressed(GLFW_KEY_SPACE) || Input::keyPressed(GLFW_KEY_ENTER))
            changeState(GameState::Playing);
        break;

    case GameState::Playing:
        if (Input::keyPressed(GLFW_KEY_ESCAPE))
            changeState(GameState::Paused);
        else
            updateGame(dt);
        drawGame();
        drawHUD();
        break;

    case GameState::Paused:
        drawGame();
        drawHUD();
        pauseMenu();
        break;

    case GameState::Won:
    case GameState::Lost:
        drawGame();
        drawHUD();
        endMenu();
        break;
    }
}

void Game::updateGame(float dt)
{
    world.time += dt;

    PlayerCards::update(world);
    Lanes::update(world);
    Players::update(world, dt);
    Waves::update(world, dt);
    Enemies::update(world, dt);
    Combat::update(world, dt);
    Collectibles::update(world, dt);

    removeDead();
    checkWinLose();
}

template <typename T>
static void eraseDead(std::vector<T>& list)
{
    list.erase(std::remove_if(list.begin(), list.end(), [](const T& item) { return !item.alive; }), list.end());
}

void Game::removeDead()
{
    eraseDead(world.players);
    eraseDead(world.enemies);
    eraseDead(world.projectiles);
    eraseDead(world.collectibles);
}

void Game::checkWinLose()
{
    for (const Enemy& e : world.enemies)
    {
        if (e.x - e.halfWidth <= HOME_BASE_RIGHT)
        {
            changeState(GameState::Lost);
            return;
        }
    }

    if (Waves::allSpawned(world) && world.enemies.empty())
        changeState(GameState::Won);
}

void Game::drawGame()
{
    Environment::drawBackground(*r, world.time);
    Lanes::draw(*r, world);
    Environment::drawHomeBase(*r, world.time);
    Collectibles::draw(*r, world);

    for (int lane = 0; lane < NUM_LANES; lane++)
    {
        for (const Player& p : world.players)
            if (p.lane == lane) Players::draw(*r, world, p);
        for (const Enemy& e : world.enemies)
            if (e.lane == lane) Enemies::draw(*r, world, e);
    }

    for (const Projectile& pr : world.projectiles)
    {
        if (pr.fromPlayer) Players::drawProjectile(*r, pr);
        else               Enemies::drawProjectile(*r, pr);
    }

    Environment::drawForeground(*r, world.time);

    for (const Player& p : world.players)
        UI::healthBar(*r, p.x, laneGroundY(p.lane) + p.y + p.height + 8, p.hp, p.maxHp);
    for (const Enemy& e : world.enemies)
        UI::healthBar(*r, e.x, laneGroundY(e.lane) + e.height + 8, e.hp, e.maxHp);
}

void Game::drawHUD()
{
    r->rectGradient(0, FIELD_TOP, (float)VIRTUAL_W, VIRTUAL_H - FIELD_TOP, Colors::rgb(40, 30, 26), Colors::rgb(22, 18, 18));
    r->rect(0, FIELD_TOP, (float)VIRTUAL_W, 4, Colors::rgb(120, 86, 52));

    r->roundedRect(9, 623, 178, 78, 12, Colors::rgb(150, 110, 70));
    r->roundedRect(12, 626, 172, 72, 10, Colors::rgb(28, 22, 22));
    UI::coinIcon(*r, 50, 662, 22);
    Text::drawShadowed(*r, std::to_string(world.coins), 84, 648, 28, Colors::rgb(255, 230, 140));
    Text::draw(*r, "COINS", 84, 634, 10, Colors::rgb(190, 160, 120));

    PlayerCards::draw(*r, world);

    float x = VIRTUAL_W - 252.0f;
    r->roundedRect(x - 3, 623, 246, 78, 12, Colors::rgb(150, 110, 70));
    r->roundedRect(x, 626, 240, 72, 10, Colors::rgb(28, 22, 22));
    std::string wave = "WAVE " + std::to_string(Waves::currentWave(world)) + "/" + std::to_string(NUM_WAVES);
    Text::drawShadowed(*r, wave, x + 120, 664, 22, Colors::rgb(230, 90, 70), Align::Center);
    Text::draw(*r, "ZOMBIES ON FIELD: " + std::to_string(world.enemies.size()), x + 120, 638, 11,
               Colors::rgb(200, 180, 160), Align::Center);

    Text::draw(*r, "CLICK LANE: SELECT   W/UP: JUMP   S/DOWN: DUCK   ESC: PAUSE",
               CENTER_X + 160, 606, 9, Colors::rgb(150, 130, 110), Align::Center);
}

void Game::menuBackground()
{
    Environment::drawBackground(*r, menuTime);
    Environment::drawHomeBase(*r, menuTime);
    r->fullscreen(Color(0.03f, 0.02f, 0.03f, 0.72f));
}

void Game::mainMenu()
{
    menuBackground();
    Text::drawShadowed(*r, GAME_TITLE, CENTER_X, 520, 72, Colors::rgb(210, 40, 30), Align::Center);
    Text::draw(*r, "HOLD THE LINE. SURVIVE THE NIGHT.", CENTER_X, 470, 16, Colors::rgb(220, 200, 170), Align::Center);

    float x = CENTER_X - BUTTON_W / 2;
    if (UI::button(*r, "START GAME", x, 360, BUTTON_W, BUTTON_H))
        newGame(true);
    if (UI::button(*r, "SOUND SETTINGS", x, 286, BUTTON_W, BUTTON_H))
        changeState(GameState::SoundSettings);
    if (UI::button(*r, "EXIT", x, 212, BUTTON_W, BUTTON_H))
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Game::soundMenu()
{
    menuBackground();
    UI::panel(*r, CENTER_X - 230, 150, 460, 420);
    Text::drawShadowed(*r, "SOUND SETTINGS", CENTER_X, 510, 32, Colors::rgb(235, 200, 140), Align::Center);

    float barX = CENTER_X - 160;
    r->rect(barX - 3, 427, 326, 32, Colors::rgb(10, 10, 10));
    r->rect(barX, 430, 320, 26, Colors::rgb(50, 40, 40));
    Color fill = Audio::muted() ? Colors::rgb(90, 90, 90) : Colors::rgb(200, 60, 45);
    r->rect(barX, 430, 320 * Audio::volume(), 26, fill);

    int percent = (int)(Audio::volume() * 100 + 0.5f);
    std::string label = Audio::muted() ? "MUTED" : "VOLUME " + std::to_string(percent) + "%";
    Text::draw(*r, label, CENTER_X, 395, 16, Colors::rgb(230, 220, 200), Align::Center);

    if (UI::button(*r, "- VOL", CENTER_X - 200, 310, 120, 52))
        Audio::volumeDown();
    if (UI::button(*r, Audio::muted() ? "UNMUTE" : "MUTE", CENTER_X - 60, 310, 120, 52))
        Audio::setMuted(!Audio::muted());
    if (UI::button(*r, "+ VOL", CENTER_X + 80, 310, 120, 52))
        Audio::volumeUp();

    if (UI::button(*r, "BACK", CENTER_X - BUTTON_W / 2, 190, BUTTON_W, BUTTON_H) || Input::keyPressed(GLFW_KEY_ESCAPE))
        changeState(GameState::MainMenu);
}

void Game::pauseMenu()
{
    r->fullscreen(Color(0, 0, 0, 0.55f));
    UI::panel(*r, CENTER_X - 200, 170, 400, 360);
    Text::drawShadowed(*r, "PAUSED", CENTER_X, 460, 40, Colors::rgb(235, 200, 140), Align::Center);

    float x = CENTER_X - BUTTON_W / 2;
    if (UI::button(*r, "CONTINUE", x, 370, BUTTON_W, BUTTON_H) || Input::keyPressed(GLFW_KEY_ESCAPE))
        changeState(GameState::Playing);
    else if (UI::button(*r, "RESTART", x, 296, BUTTON_W, BUTTON_H))
        newGame(false);
    else if (UI::button(*r, "MAIN MENU", x, 222, BUTTON_W, BUTTON_H))
        changeState(GameState::MainMenu);
}

void Game::endMenu()
{
    bool won = state == GameState::Won;
    r->fullscreen(won ? Color(0, 0.05f, 0.02f, 0.6f) : Color(0.15f, 0, 0, 0.65f));
    UI::panel(*r, CENTER_X - 260, 170, 520, 380);

    if (won)
    {
        Text::drawShadowed(*r, "YOU SURVIVED!", CENTER_X, 470, 44, Colors::rgb(120, 230, 120), Align::Center);
        Text::draw(*r, "ALL THREE WAVES WERE PUSHED BACK.", CENTER_X, 420, 14, Colors::rgb(220, 210, 190), Align::Center);
    }
    else
    {
        Text::drawShadowed(*r, "THE BASE HAS FALLEN", CENTER_X, 470, 38, Colors::rgb(235, 60, 45), Align::Center);
        Text::draw(*r, "A ZOMBIE BROKE THROUGH THE LINE.", CENTER_X, 420, 14, Colors::rgb(220, 210, 190), Align::Center);
    }

    float x = CENTER_X - BUTTON_W / 2;
    if (UI::button(*r, "RESTART", x, 310, BUTTON_W, BUTTON_H))
        newGame(false);
    else if (UI::button(*r, "MAIN MENU", x, 236, BUTTON_W, BUTTON_H))
        changeState(GameState::MainMenu);
}

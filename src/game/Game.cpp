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

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    const float kBtnW = 260.0f, kBtnH = 56.0f, kBtnGap = 18.0f;
    const float kCenterX = VIRTUAL_W * 0.5f;

    template <typename T>
    void eraseDead(std::vector<T>& v)
    {
        v.erase(std::remove_if(v.begin(), v.end(), [](const T& e) { return !e.alive; }), v.end());
    }
}

// ============================================================================
//  lifecycle
// ============================================================================

bool Game::init(GLFWwindow* window, Renderer2D* renderer)
{
    m_window = window;
    m_r = renderer;
    m_world.reset();
    setState(GameState::MainMenu);
    return true;
}

void Game::shutdown() {}

void Game::setState(GameState s)
{
    m_state = s;
    m_stateTime = 0.0f;

    switch (s)
    {
    case GameState::MainMenu:
    case GameState::SoundSettings:
    case GameState::Cutscene:
        Audio::playMusic(Music::Menu);
        break;
    case GameState::Playing:
        Audio::playMusic(Music::Gameplay);
        break;
    case GameState::Won:
        Audio::stopMusic();
        Audio::play(Sfx::Win);
        break;
    case GameState::Lost:
        Audio::stopMusic();
        Audio::play(Sfx::Lose);
        break;
    case GameState::Paused:
        break;
    }
}

void Game::startNewGame(bool withCutscene)
{
    m_world.reset();
    m_shownCoins = m_world.coins;
    m_coinPulse = 0.0f;
    m_lastWave = 0;
    m_bannerTime = 0.0f;
    if (withCutscene)
    {
        Cutscene::reset();
        setState(GameState::Cutscene);
    }
    else
    {
        setState(GameState::Playing);
    }
}

// ============================================================================
//  per-frame
// ============================================================================

void Game::frame(float dt)
{
    m_stateTime += dt;
    m_menuTime += dt;
    Renderer2D& r = *m_r;

    switch (m_state)
    {
    case GameState::MainMenu:
        mainMenu();
        break;

    case GameState::SoundSettings:
        soundSettings();
        break;

    case GameState::Cutscene:
    {
        Cutscene::update(dt);
        Cutscene::draw(r);
        Text::drawShadowed(r, "SPACE / ENTER TO SKIP", VIRTUAL_W - 20.0f, 16.0f, 14,
                           Color(1, 1, 1, 0.6f), Align::Right);
        bool skip = Input::keyPressed(GLFW_KEY_SPACE) || Input::keyPressed(GLFW_KEY_ENTER);
        if (Cutscene::finished() || skip)
            setState(GameState::Playing);
        break;
    }

    case GameState::Playing:
        if (Input::keyPressed(GLFW_KEY_ESCAPE))
        {
            setState(GameState::Paused);
            drawScene();
            drawHUD();
            break;
        }
        updatePlaying(dt);
        drawScene();
        drawHUD();
        drawWaveBanner();
        break;

    case GameState::Paused:
        // Nothing updates: the scene is frozen behind the menu.
        drawScene();
        drawHUD();
        pauseMenu();
        break;

    case GameState::Won:
    case GameState::Lost:
        // Spec 3.4: on loss everything stops - we simply stop updating the world.
        drawScene();
        drawHUD();
        endMenu(m_state == GameState::Won);
        break;
    }
}

// ============================================================================
//  gameplay
// ============================================================================

void Game::updatePlaying(float dt)
{
    World& w = m_world;
    w.time += dt;

    // UI first so it can consume clicks before the lanes see them.
    PlayerCards::update(w);
    Lanes::update(w);

    Players::update(w, dt);
    Waves::update(w, dt);
    Enemies::update(w, dt);
    Combat::update(w, dt);
    Collectibles::update(w, dt);

    removeDead();
    checkWinLose();

    // HUD animation bookkeeping
    if (w.coins != m_shownCoins) { m_coinPulse = 1.0f; m_shownCoins = w.coins; }
    m_coinPulse = std::max(0.0f, m_coinPulse - dt * 3.0f);

    int wave = Waves::currentWave(w);
    if (wave != m_lastWave) { m_lastWave = wave; m_bannerTime = 2.2f; }
    m_bannerTime = std::max(0.0f, m_bannerTime - dt);
}

void Game::removeDead()
{
    eraseDead(m_world.players);
    eraseDead(m_world.enemies);
    eraseDead(m_world.projectiles);
    eraseDead(m_world.collectibles);
}

void Game::checkWinLose()
{
    // Lose: any zombie reaches the home base, from any lane.
    for (const Enemy& e : m_world.enemies)
    {
        if (e.alive && e.x - e.halfWidth <= HOME_BASE_RIGHT)
        {
            setState(GameState::Lost);
            return;
        }
    }
    // Win: every wave has fully spawned and no zombie is left alive.
    if (Waves::allSpawned(m_world) && m_world.enemies.empty())
        setState(GameState::Won);
}

// ============================================================================
//  drawing
// ============================================================================

void Game::drawScene()
{
    Renderer2D& r = *m_r;
    const World& w = m_world;

    Environment::drawBackground(r, w.time);
    Lanes::draw(r, w);
    Environment::drawHomeBase(r, w.time);

    Collectibles::draw(r, w);

    // Draw lane by lane from the top lane down so lower lanes overlap upper ones.
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        for (const Player& p : w.players)
            if (p.lane == lane) Players::draw(r, w, p);
        for (const Enemy& e : w.enemies)
            if (e.lane == lane) Enemies::draw(r, w, e);
    }

    for (const Projectile& pr : w.projectiles)
    {
        if (!pr.alive) continue;
        if (pr.fromPlayer) Players::drawProjectile(r, pr);
        else               Enemies::drawProjectile(r, pr);
    }

    Environment::drawForeground(r, w.time);
    drawHealthBars();
}

void Game::drawHealthBars()
{
    Renderer2D& r = *m_r;
    for (const Player& p : m_world.players)
        if (p.alive)
            UI::healthBar(r, p.x, laneGroundY(p.lane) + p.y + p.height + 8.0f, p.hp, p.maxHp);
    for (const Enemy& e : m_world.enemies)
        if (e.alive)
            UI::healthBar(r, e.x, laneGroundY(e.lane) + e.height + 8.0f, e.hp, e.maxHp);
}

void Game::drawHUD()
{
    Renderer2D& r = *m_r;
    const World& w = m_world;

    // Top bar background
    r.rectGradient(0, FIELD_TOP, (float)VIRTUAL_W, VIRTUAL_H - FIELD_TOP,
                   Colors::rgb(40, 30, 26), Colors::rgb(22, 18, 18));
    r.rect(0, FIELD_TOP, (float)VIRTUAL_W, 4, Colors::rgb(120, 86, 52));
    r.rect(0, FIELD_TOP + 4, (float)VIRTUAL_W, 2, Colors::rgb(10, 8, 8));

    // ---- coin counter (top-left) ----
    float px = 12, py = 626, pw = 172, ph = 72;
    r.roundedRect(px - 3, py - 3, pw + 6, ph + 6, 12, Colors::rgb(150, 110, 70));
    r.roundedRect(px, py, pw, ph, 10, Colors::rgb(28, 22, 22));
    float pop = 1.0f + 0.25f * m_coinPulse;
    UI::coinIcon(r, px + 38, py + ph * 0.5f, 22 * pop);
    Text::drawShadowed(r, std::to_string(w.coins), px + 72, py + ph * 0.5f - 14, 28 * (1.0f + 0.1f * m_coinPulse),
                       Colors::lerp(Colors::rgb(255, 230, 140), Colors::White, m_coinPulse));
    Text::draw(r, "COINS", px + 72, py + 8, 10, Colors::rgb(190, 160, 120));

    // ---- player cards (Member 2) ----
    PlayerCards::draw(r, w);

    // ---- wave info (top-right) ----
    float wx = VIRTUAL_W - 252.0f, wy = 626, ww = 240, wh = 72;
    r.roundedRect(wx - 3, wy - 3, ww + 6, wh + 6, 12, Colors::rgb(150, 110, 70));
    r.roundedRect(wx, wy, ww, wh, 10, Colors::rgb(28, 22, 22));
    std::string waveText = "WAVE " + std::to_string(Waves::currentWave(w)) + "/" + std::to_string(NUM_WAVES);
    Text::drawShadowed(r, waveText, wx + ww * 0.5f, wy + 38, 22, Colors::rgb(230, 90, 70), Align::Center);
    std::string left = "ZOMBIES ON FIELD: " + std::to_string(w.enemies.size());
    Text::draw(r, left, wx + ww * 0.5f, wy + 12, 11, Colors::rgb(200, 180, 160), Align::Center);

    // ---- controls hint ----
    Text::draw(r, "CLICK LANE: SELECT   W/UP: JUMP   S/DOWN: DUCK   ESC: PAUSE",
               kCenterX + 160, 606 + 0.0f, 9, Colors::rgb(150, 130, 110), Align::Center);
}

void Game::drawWaveBanner()
{
    if (m_bannerTime <= 0.0f) return;
    Renderer2D& r = *m_r;
    float t = m_bannerTime;
    float a = std::min(1.0f, t / 0.4f) * std::min(1.0f, (2.2f - t) / 0.3f + 0.01f);
    a = std::clamp(a, 0.0f, 1.0f);

    r.push();
    r.setAlpha(a);
    r.rect(0, 300, (float)VIRTUAL_W, 90, Color(0, 0, 0, 0.55f));
    std::string s = "WAVE " + std::to_string(m_lastWave);
    if (m_lastWave == NUM_WAVES) s += " - FINAL WAVE";
    Text::drawShadowed(r, s, kCenterX, 326, 40, Colors::rgb(235, 70, 50), Align::Center);
    r.pop();
}

// ============================================================================
//  menus
// ============================================================================

void Game::menuBackdrop()
{
    Renderer2D& r = *m_r;
    Environment::drawBackground(r, m_menuTime);
    Environment::drawHomeBase(r, m_menuTime);
    r.fullscreen(Color(0.03f, 0.02f, 0.03f, 0.72f));
    // vignette bars
    r.rectGradient(0, 0, (float)VIRTUAL_W, 160, Color(0, 0, 0, 0.6f), Color(0, 0, 0, 0));
    r.rectGradient(0, VIRTUAL_H - 160.0f, (float)VIRTUAL_W, 160, Color(0, 0, 0, 0), Color(0, 0, 0, 0.6f));
}

void Game::mainMenu()
{
    Renderer2D& r = *m_r;
    menuBackdrop();

    // Title with a slow "flicker" like a failing neon sign
    float flicker = (std::sin(m_menuTime * 13.0f) > 0.96f) ? 0.55f : 1.0f;
    Text::drawShadowed(r, GAME_TITLE, kCenterX, 520, 72, Colors::rgb(210, 40, 30) * flicker + Color(0, 0, 0, 1 - flicker),
                       Align::Center, Colors::rgb(0, 0, 0, 200));
    Text::draw(r, "HOLD THE LINE. SURVIVE THE NIGHT.", kCenterX, 470, 16, Colors::rgb(220, 200, 170), Align::Center);

    float x = kCenterX - kBtnW * 0.5f;
    float y = 360;
    if (UI::button(r, "START GAME", x, y, kBtnW, kBtnH))                       startNewGame(true);
    if (UI::button(r, "SOUND SETTINGS", x, y - (kBtnH + kBtnGap), kBtnW, kBtnH)) setState(GameState::SoundSettings);
    if (UI::button(r, "EXIT", x, y - 2 * (kBtnH + kBtnGap), kBtnW, kBtnH))     glfwSetWindowShouldClose(m_window, GLFW_TRUE);

    Text::draw(r, "DMET 502 - COMPUTER GRAPHICS - WINTER 2026", kCenterX, 24, 10,
               Colors::rgb(140, 120, 100), Align::Center);
}

void Game::soundSettings()
{
    Renderer2D& r = *m_r;
    menuBackdrop();
    UI::panel(r, kCenterX - 230, 150, 460, 420);
    Text::drawShadowed(r, "SOUND SETTINGS", kCenterX, 510, 32, Colors::rgb(235, 200, 140), Align::Center);

    // Volume bar
    float barX = kCenterX - 160, barY = 430, barW = 320, barH = 26;
    r.rect(barX - 3, barY - 3, barW + 6, barH + 6, Colors::rgb(10, 10, 10));
    r.rect(barX, barY, barW, barH, Colors::rgb(50, 40, 40));
    Color fill = Audio::muted() ? Colors::rgb(90, 90, 90) : Colors::rgb(200, 60, 45);
    r.rect(barX, barY, barW * Audio::volume(), barH, fill);
    for (int i = 1; i < 10; ++i) r.rect(barX + barW * i / 10.0f - 1, barY, 2, barH, Colors::rgb(10, 10, 10, 180));

    std::string label = Audio::muted() ? "MUTED" : ("VOLUME " + std::to_string((int)std::lround(Audio::volume() * 100)) + "%");
    Text::draw(r, label, kCenterX, 395, 16, Colors::rgb(230, 220, 200), Align::Center);

    float bw = 120, bh = 52;
    if (UI::button(r, "- VOL", kCenterX - 200, 310, bw, bh)) Audio::volumeDown();
    if (UI::button(r, Audio::muted() ? "UNMUTE" : "MUTE", kCenterX - bw * 0.5f, 310, bw, bh))
        Audio::setMuted(!Audio::muted());
    if (UI::button(r, "+ VOL", kCenterX + 80, 310, bw, bh)) Audio::volumeUp();

    if (UI::button(r, "BACK", kCenterX - kBtnW * 0.5f, 190, kBtnW, kBtnH) || Input::keyPressed(GLFW_KEY_ESCAPE))
        setState(GameState::MainMenu);
}

void Game::pauseMenu()
{
    Renderer2D& r = *m_r;
    r.fullscreen(Color(0, 0, 0, 0.55f));
    UI::panel(r, kCenterX - 200, 170, 400, 360);
    Text::drawShadowed(r, "PAUSED", kCenterX, 460, 40, Colors::rgb(235, 200, 140), Align::Center);

    float x = kCenterX - kBtnW * 0.5f;
    if (UI::button(r, "CONTINUE", x, 370, kBtnW, kBtnH) || (m_stateTime > 0.0f && Input::keyPressed(GLFW_KEY_ESCAPE)))
        setState(GameState::Playing);
    else if (UI::button(r, "RESTART", x, 296, kBtnW, kBtnH))
        startNewGame(false);
    else if (UI::button(r, "MAIN MENU", x, 222, kBtnW, kBtnH))
        setState(GameState::MainMenu);
}

void Game::endMenu(bool won)
{
    Renderer2D& r = *m_r;
    float fade = std::min(1.0f, m_stateTime / 0.6f);
    r.fullscreen(won ? Color(0.0f, 0.05f, 0.02f, 0.6f * fade) : Color(0.15f, 0.0f, 0.0f, 0.65f * fade));
    if (m_stateTime < 0.6f) return;   // brief beat so the player sees what happened

    UI::panel(r, kCenterX - 260, 170, 520, 380);
    if (won)
    {
        Text::drawShadowed(r, "YOU SURVIVED!", kCenterX, 470, 44, Colors::rgb(120, 230, 120), Align::Center);
        Text::draw(r, "ALL THREE WAVES WERE PUSHED BACK.", kCenterX, 420, 14, Colors::rgb(220, 210, 190), Align::Center);
    }
    else
    {
        Text::drawShadowed(r, "THE BASE HAS FALLEN", kCenterX, 470, 38, Colors::rgb(235, 60, 45), Align::Center);
        Text::draw(r, "A ZOMBIE BROKE THROUGH THE LINE.", kCenterX, 420, 14, Colors::rgb(220, 210, 190), Align::Center);
    }

    float x = kCenterX - kBtnW * 0.5f;
    if (UI::button(r, "RESTART", x, 310, kBtnW, kBtnH))
        startNewGame(false);
    else if (UI::button(r, "MAIN MENU", x, 236, kBtnW, kBtnH))
        setState(GameState::MainMenu);
}

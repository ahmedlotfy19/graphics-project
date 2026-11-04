#pragma once

namespace Sfx
{
    constexpr const char* Click         = "click";
    constexpr const char* LaneSelect    = "lane_select";
    constexpr const char* Place         = "place";
    constexpr const char* PlayerAttack  = "player_attack";
    constexpr const char* EnemyAttack   = "enemy_attack";
    constexpr const char* Hit           = "hit";
    constexpr const char* PlayerDeath   = "player_death";
    constexpr const char* EnemyDeath    = "enemy_death";
    constexpr const char* Coin          = "coin";
    constexpr const char* Jump          = "jump";
    constexpr const char* Win           = "win";
    constexpr const char* Lose          = "lose";
}

namespace Music
{
    constexpr const char* Menu     = "music_menu";
    constexpr const char* Gameplay = "music_game";
}

namespace Audio
{
    bool  init();
    void  shutdown();

    void  play(const char* sfxName);
    void  playMusic(const char* musicName);
    void  stopMusic();

    void  setVolume(float v);
    float volume();
    void  volumeUp();
    void  volumeDown();
    void  setMuted(bool m);
    bool  muted();
}

#pragma once

namespace Sfx
{
    const char* const Click        = "click";
    const char* const LaneSelect   = "lane_select";
    const char* const Place        = "place";
    const char* const PlayerAttack = "player_attack";
    const char* const EnemyAttack  = "enemy_attack";
    const char* const Hit          = "hit";
    const char* const PlayerDeath  = "player_death";
    const char* const EnemyDeath   = "enemy_death";
    const char* const Coin         = "coin";
    const char* const Jump         = "jump";
    const char* const Win          = "win";
    const char* const Lose         = "lose";
}

namespace Music
{
    const char* const Menu     = "music_menu";
    const char* const Gameplay = "music_game";
}

namespace Audio
{
    bool  init();
    void  shutdown();

    void  play(const char* name);
    void  playMusic(const char* name);
    void  stopMusic();

    void  volumeUp();
    void  volumeDown();
    float volume();
    void  setMuted(bool m);
    bool  muted();
}

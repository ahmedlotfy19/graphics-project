#pragma once
// ============================================================================
//  Audio  -  music + sound effects (bonus), using miniaudio (single header).
//  Owner: Member 1 (Core).
//
//      Audio::play(Sfx::Coin);
//      Audio::playMusic(Music::Gameplay);
//
//  Sound files live in  assets/audio/<name>.wav  (or .mp3 / .flac).
//  A missing file is NOT an error: it prints one warning and stays silent,
//  so the game always runs even before real sounds are added.
//  To add a new effect: add a name to the Sfx namespace and drop the file in.
// ============================================================================

namespace Sfx
{
    constexpr const char* Click         = "click";          // menu buttons
    constexpr const char* LaneSelect    = "lane_select";
    constexpr const char* Place         = "place";          // player placed
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
    void  playMusic(const char* musicName);   // loops; no-op if already playing it
    void  stopMusic();

    void  setVolume(float v);                 // 0..1, master
    float volume();
    void  volumeUp();                         // +10%
    void  volumeDown();                       // -10%
    void  setMuted(bool m);
    bool  muted();
}

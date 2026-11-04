#pragma once
// ============================================================================
//  Waves  -  enemy spawning across the 3 waves.     Owner: MEMBER 3
//  Spec 3.3: wave 1 = 3 easy, wave 2 = 5 easy/medium, wave 3 = 10 mixed.
//  A wave "ends" when its last enemy spawns; the next wave starts only after
//  every enemy on screen is dead.
// ============================================================================

struct World;

struct WaveState
{
    int   index = 0;          // current wave 0..NUM_WAVES-1
    int   spawned = 0;        // enemies spawned in the current wave
    float timer = 2.0f;       // countdown to next spawn
    bool  betweenWaves = false;
    bool  finished = false;   // true once the LAST enemy of the LAST wave spawned
};

namespace Waves
{
    void update(World& w, float dt);
    int  currentWave(const World& w);        // 1-based, for the HUD
    bool allSpawned(const World& w);         // Core uses this + "no enemies left" to detect a win
}

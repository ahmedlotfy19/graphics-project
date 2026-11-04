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
    int   index = 0;
    int   spawned = 0;
    float timer = 2.0f;
    bool  betweenWaves = false;
    bool  finished = false;
};

namespace Waves
{
    void update(World& w, float dt);
    int  currentWave(const World& w);
    bool allSpawned(const World& w);
}

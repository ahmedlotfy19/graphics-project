#pragma once
// ============================================================================
//  Waves  -  enemy spawning across the 3 waves.     Owner: MEMBER 3
//  Wave 1 = easy only, wave 2 = easy + medium, wave 3 = easy + medium + hard,
//  sizes from WAVE_ENEMY_COUNT in Config.h, shuffled, random lanes.
//  A wave "ends" when its last zombie spawns; the next wave starts only after
//  every zombie on screen is dead.
// ============================================================================
#include "Enemies.h"
#include <vector>

struct World;

struct WaveState
{
    int   index = 0;
    int   spawned = 0;
    float timer = 2.5f;
    bool  betweenWaves = false;
    bool  finished = false;
    int   lastLane = -1;
    std::vector<EnemyType> queue;
};

namespace Waves
{
    void update(World& w, float dt);
    int  currentWave(const World& w);
    bool allSpawned(const World& w);
}

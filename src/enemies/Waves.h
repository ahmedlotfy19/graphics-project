#pragma once
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
    std::vector<EnemyType> queue;
};

namespace Waves
{
    void update(World& w, float dt);
    int  currentWave(const World& w);
    bool allSpawned(const World& w);
}

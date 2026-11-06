#include "Waves.h"
#include "../game/World.h"

#include <algorithm>
#include <cstdlib>

static std::vector<EnemyType> makeWave(int wave)
{
    int total = WAVE_ENEMY_COUNT[wave];
    int medium = 0;
    int hard = 0;

    if (wave == 1)
    {
        medium = total * 2 / 5;
    }
    else if (wave == 2)
    {
        medium = total * 3 / 10;
        hard = total * 3 / 10;
    }

    std::vector<EnemyType> list;
    for (int i = 0; i < total - medium - hard; i++) list.push_back(EnemyType::UpperRanged);
    for (int i = 0; i < medium; i++) list.push_back(EnemyType::LowerRanged);
    for (int i = 0; i < hard; i++) list.push_back(EnemyType::Melee);

    for (int i = (int)list.size() - 1; i > 0; i--)
        std::swap(list[i], list[rand() % (i + 1)]);

    return list;
}

static float randomFloat(float a, float b)
{
    return a + (b - a) * (rand() / (float)RAND_MAX);
}

namespace Waves
{
    void update(World& w, float dt)
    {
        WaveState& s = w.waves;
        if (s.finished) return;

        if (s.betweenWaves)
        {
            if (!w.enemies.empty()) return;

            s.timer -= dt;
            if (s.timer > 0) return;

            s.betweenWaves = false;
            s.index++;
            s.spawned = 0;
            s.queue.clear();
            s.timer = 0.5f;
        }

        if (s.queue.empty())
            s.queue = makeWave(s.index);

        s.timer -= dt;
        if (s.timer > 0) return;

        Enemies::spawn(w, s.queue[s.spawned], rand() % NUM_LANES);
        s.spawned++;

        float minGap[] = { 4.5f, 3.5f, 2.5f };
        float maxGap[] = { 6.5f, 5.5f, 4.2f };
        s.timer = randomFloat(minGap[s.index], maxGap[s.index]);

        if (s.spawned == (int)s.queue.size())
        {
            if (s.index == NUM_WAVES - 1)
            {
                s.finished = true;
            }
            else
            {
                s.betweenWaves = true;
                s.timer = 4;
            }
        }
    }

    int currentWave(const World& w)
    {
        return w.waves.index + 1;
    }

    bool allSpawned(const World& w)
    {
        return w.waves.finished;
    }
}

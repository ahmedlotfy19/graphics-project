#include "Waves.h"
#include "../game/World.h"

#include <algorithm>
#include <cstdlib>

namespace
{
    const float kMinGap[NUM_WAVES] = { 4.5f, 3.5f, 2.5f };
    const float kMaxGap[NUM_WAVES] = { 6.5f, 5.5f, 4.2f };
    const float kBreakBetweenWaves = 4.0f;

    float randRange(float a, float b) { return a + (b - a) * (std::rand() / (float)RAND_MAX); }

    std::vector<EnemyType> buildQueue(int wave)
    {
        int total = WAVE_ENEMY_COUNT[wave];
        int medium = 0, hard = 0;
        if (wave == 1) medium = std::max(1, total * 2 / 5);
        if (wave >= 2) { medium = std::max(1, total * 3 / 10); hard = std::max(1, total * 3 / 10); }

        std::vector<EnemyType> q;
        for (int i = 0; i < total - medium - hard; ++i) q.push_back(EnemyType::UpperRanged);
        for (int i = 0; i < medium; ++i) q.push_back(EnemyType::LowerRanged);
        for (int i = 0; i < hard; ++i) q.push_back(EnemyType::Melee);

        for (int i = (int)q.size() - 1; i > 0; --i)
            std::swap(q[i], q[std::rand() % (i + 1)]);
        return q;
    }

    int pickLane(WaveState& s)
    {
        int lane = std::rand() % NUM_LANES;
        if (lane == s.lastLane && NUM_LANES > 1)
            lane = (lane + 1 + std::rand() % (NUM_LANES - 1)) % NUM_LANES;
        s.lastLane = lane;
        return lane;
    }
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
            if (s.timer > 0.0f) return;
            s.betweenWaves = false;
            s.index++;
            s.spawned = 0;
            s.queue.clear();
            s.timer = 0.5f;
            return;
        }

        if (s.queue.empty()) s.queue = buildQueue(s.index);

        s.timer -= dt;
        if (s.timer > 0.0f) return;

        Enemies::spawn(w, s.queue[s.spawned], pickLane(s));
        s.spawned++;
        s.timer = randRange(kMinGap[s.index], kMaxGap[s.index]);

        if (s.spawned >= (int)s.queue.size())
        {
            if (s.index == NUM_WAVES - 1) s.finished = true;
            else { s.betweenWaves = true; s.timer = kBreakBetweenWaves; }
        }
    }

    int  currentWave(const World& w) { return w.waves.index + 1; }
    bool allSpawned(const World& w)  { return w.waves.finished; }
}

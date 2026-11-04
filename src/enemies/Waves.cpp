// ============================================================================
//  PLACEHOLDER - Member 3 refines (pacing, mixes, random lanes are already in).
// ============================================================================
#include "Waves.h"
#include "../game/World.h"

#include <cstdlib>

namespace
{
    float randRange(float a, float b) { return a + (b - a) * (std::rand() / (float)RAND_MAX); }

    EnemyType pickType(int wave)
    {
        int roll = std::rand() % 100;
        if (wave == 0) return EnemyType::UpperRanged;                                    // easy only
        if (wave == 1) return roll < 55 ? EnemyType::UpperRanged : EnemyType::LowerRanged; // easy + medium
        if (roll < 35) return EnemyType::UpperRanged;                                    // all three
        if (roll < 70) return EnemyType::LowerRanged;
        return EnemyType::Melee;
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
            // Next wave only once the field is clear.
            if (!w.enemies.empty()) return;
            s.timer -= dt;
            if (s.timer > 0.0f) return;
            s.betweenWaves = false;
            s.index++;
            s.spawned = 0;
            s.timer = 0.5f;
            return;
        }

        s.timer -= dt;
        if (s.timer > 0.0f) return;

        Enemies::spawn(w, pickType(s.index), std::rand() % NUM_LANES);
        s.spawned++;
        s.timer = randRange(3.0f, 6.0f) - s.index * 0.8f;

        if (s.spawned >= WAVE_ENEMY_COUNT[s.index])
        {
            if (s.index == NUM_WAVES - 1) s.finished = true;
            else { s.betweenWaves = true; s.timer = 3.0f; }
        }
    }

    int currentWave(const World& w) { return w.waves.index + 1; }
    bool allSpawned(const World& w) { return w.waves.finished; }
}

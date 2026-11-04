#pragma once
// ============================================================================
//  World  -  ALL gameplay state lives here. Passed by reference to every system.
//  Owner: Member 1. Each member owns the struct type of their own field
//  (Player/Enemy/LaneState/CardState/WaveState), defined in their own headers,
//  so you can add fields there without touching this file.
// ============================================================================
#include "../Config.h"
#include "Projectile.h"
#include "../players/Players.h"
#include "../players/PlayerCards.h"
#include "../enemies/Enemies.h"
#include "../enemies/Waves.h"
#include "../world/Lanes.h"

#include <vector>

struct World
{
    std::vector<Player>      players;
    std::vector<Enemy>       enemies;
    std::vector<Projectile>  projectiles;
    std::vector<Collectible> collectibles;

    int   coins = START_COINS;
    float time = 0.0f;           // seconds of gameplay (stops while paused)

    LaneState lanes;             // Member 4
    CardState cards;             // Member 2
    WaveState waves;             // Member 3

    void reset() { *this = World(); }
};

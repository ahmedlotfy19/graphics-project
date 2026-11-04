#pragma once
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
    float time = 0.0f;

    LaneState lanes;
    CardState cards;
    WaveState waves;

    void reset() { *this = World(); }
};

#pragma once

constexpr int   VIRTUAL_W = 1280;
constexpr int   VIRTUAL_H = 720;
constexpr const char* GAME_TITLE = "ZOMBIE LAND";

constexpr int   NUM_LANES       = 4;
constexpr float LANE_HEIGHT     = 145.0f;
constexpr float FIELD_TOP       = 600.0f;
constexpr float FIELD_BOTTOM    = FIELD_TOP - NUM_LANES * LANE_HEIGHT;
constexpr float HOME_BASE_RIGHT = 150.0f;
constexpr float FIELD_RIGHT     = (float)VIRTUAL_W;
constexpr float GROUND_OFFSET   = 22.0f;

constexpr float PLAYER_SPAWN_X  = HOME_BASE_RIGHT + 40.0f;
constexpr float ENEMY_SPAWN_X   = FIELD_RIGHT - 30.0f;

inline float laneBottomY(int lane) { return FIELD_TOP - (lane + 1) * LANE_HEIGHT; }
inline float laneTopY(int lane)    { return FIELD_TOP - lane * LANE_HEIGHT; }
inline float laneCenterY(int lane) { return laneBottomY(lane) + LANE_HEIGHT * 0.5f; }
inline float laneGroundY(int lane) { return laneBottomY(lane) + GROUND_OFFSET; }

inline int laneAtY(float y)
{
    if (y < FIELD_BOTTOM || y >= FIELD_TOP) return -1;
    return (int)((FIELD_TOP - y) / LANE_HEIGHT);
}

constexpr int MELEE_COST        = 10;
constexpr int RANGED_COST       = 2 * MELEE_COST;
constexpr int DEFENSE_COST      = 3 * MELEE_COST;
constexpr int START_COINS       = 3 * MELEE_COST;
constexpr int COIN_DROP_VALUE   = 20;

constexpr int MELEE_PLAYER_HP    = 5;
constexpr int RANGED_PLAYER_HP   = 5;
constexpr int DEFENSE_PLAYER_HP  = 7;
constexpr int EASY_ENEMY_HP      = 5;
constexpr int MEDIUM_ENEMY_HP    = 5;
constexpr int HARD_ENEMY_HP      = 7;
constexpr int PLAYER_DAMAGE      = 1;
constexpr int ENEMY_DAMAGE       = 1;
constexpr int HARD_ENEMY_DAMAGE  = 2;

constexpr int NUM_WAVES = 3;
constexpr int WAVE_ENEMY_COUNT[NUM_WAVES] = { 3, 5, 10 };

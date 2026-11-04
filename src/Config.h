#pragma once
// ============================================================================
//  Config.h  -  shared constants for the whole game.
//  Owner: Member 1 (Core). Anyone may TUNE numbers here, but tell the team
//  before renaming/removing a constant because other files use them.
// ============================================================================

// ---- Window / virtual resolution -------------------------------------------
// All drawing uses a fixed 1280x720 "virtual" coordinate system.
// Origin (0,0) is the BOTTOM-LEFT corner, +x is right, +y is UP.
// The renderer letterboxes this onto any window size automatically.
constexpr int   VIRTUAL_W = 1280;
constexpr int   VIRTUAL_H = 720;
constexpr const char* GAME_TITLE = "LAST STAND";   // rename once the story is final

// ---- Playfield layout --------------------------------------------------------
constexpr int   NUM_LANES       = 4;       // spec: at least 3
constexpr float LANE_HEIGHT     = 145.0f;
constexpr float FIELD_TOP       = 600.0f;  // HUD lives above this line
constexpr float FIELD_BOTTOM    = FIELD_TOP - NUM_LANES * LANE_HEIGHT; // = 20
constexpr float HOME_BASE_RIGHT = 150.0f;  // home base occupies x in [0, 150]
constexpr float FIELD_RIGHT     = (float)VIRTUAL_W;
constexpr float GROUND_OFFSET   = 22.0f;   // feet height above a lane's bottom edge

constexpr float PLAYER_SPAWN_X  = HOME_BASE_RIGHT + 40.0f;  // left-most spot of a lane
constexpr float ENEMY_SPAWN_X   = FIELD_RIGHT - 30.0f;      // right-most edge

// Lane 0 is the TOP lane, lane NUM_LANES-1 is the bottom lane.
inline float laneBottomY(int lane) { return FIELD_TOP - (lane + 1) * LANE_HEIGHT; }
inline float laneTopY(int lane)    { return FIELD_TOP - lane * LANE_HEIGHT; }
inline float laneCenterY(int lane) { return laneBottomY(lane) + LANE_HEIGHT * 0.5f; }
inline float laneGroundY(int lane) { return laneBottomY(lane) + GROUND_OFFSET; }

// Returns the lane under a virtual y coordinate, or -1 if none.
inline int laneAtY(float y)
{
    if (y < FIELD_BOTTOM || y >= FIELD_TOP) return -1;
    return (int)((FIELD_TOP - y) / LANE_HEIGHT);
}

// ---- Economy (spec section 3.2) -------------------------------------------------
// Start coins buy at least 3 melee players. Ranged = 2x melee, Defense = 3x melee.
constexpr int MELEE_COST        = 10;
constexpr int RANGED_COST       = 2 * MELEE_COST;
constexpr int DEFENSE_COST      = 3 * MELEE_COST;
constexpr int START_COINS       = 3 * MELEE_COST;
constexpr int COIN_DROP_VALUE   = 10;      // value of one collectible dropped by an enemy

// ---- Health (spec tables) -----------------------------------------------------
constexpr int MELEE_PLAYER_HP    = 5;
constexpr int RANGED_PLAYER_HP   = 5;
constexpr int DEFENSE_PLAYER_HP  = 7;
constexpr int EASY_ENEMY_HP      = 5;   // upper-ranged
constexpr int MEDIUM_ENEMY_HP    = 5;   // lower-ranged
constexpr int HARD_ENEMY_HP      = 7;   // melee
constexpr int PLAYER_DAMAGE      = 1;
constexpr int ENEMY_DAMAGE       = 1;
constexpr int HARD_ENEMY_DAMAGE  = 2;

// ---- Waves (spec 3.3) ----------------------------------------------------------
constexpr int NUM_WAVES = 3;
constexpr int WAVE_ENEMY_COUNT[NUM_WAVES] = { 3, 5, 10 };

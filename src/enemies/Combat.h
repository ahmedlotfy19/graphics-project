#pragma once
// ============================================================================
//  Combat  -  collisions, "both stop and fight", projectile hits,
//             High/Low dodging, damage values.        Owner: MEMBER 3
//
//   * Moves ALL projectiles. Player bolts are hit-tested in Players::update (Member 2).
//   * Zombie projectiles: SPIT is High (duck to dodge), SKULL is Low (jump to dodge).
//     Defense players never dodge; they absorb hits for everyone behind them.
//   * Melee players hit the zombie they touch once per second and reset
//     Player::attackTimer to 1.0 (Member 2's axe animation syncs to that).
//   * Brute zombies hit the player they touch for 2.
// ============================================================================
struct World;

namespace Combat
{
    void update(World& w, float dt);
}

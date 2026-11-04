#pragma once

enum class AttackHeight { High, Low, Mid };

struct Projectile
{
    bool  fromPlayer = true;
    int   lane = 0;
    float x = 0.0f, y = 0.0f;
    float vx = 0.0f, vy = 0.0f;
    int   damage = 1;
    AttackHeight height = AttackHeight::Mid;
    int   kind = 0;
    float age = 0.0f;
    bool  alive = true;
};

struct Collectible
{
    int   lane = 0;
    float x = 0.0f;
    float y = 0.0f;
    float vy = 0.0f;
    int   value = 0;
    float age = 0.0f;
    bool  alive = true;
};

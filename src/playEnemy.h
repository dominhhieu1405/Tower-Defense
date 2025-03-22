//
// Created by ADMIN on 21/03/2025.
//
#include "Enemy.h"

#ifndef TOWER_DEFENSE_PLAYENEMY_H
#define TOWER_DEFENSE_PLAYENEMY_H

struct playEnemy {
    Enemy* enemy;
    int id;
    int level; // Level của enemy, từ 0 -> 2
    int health;
    int maxHealth;
    double speed;
    int reward;

    int tileX; // Tile x, từ 0 -> 19
    int tileY; // Tile y, từ 0 -> 10
    int x;
    int y;
    int path;
    int pathIndex = 0;
    int rotation = 90; // Góc quay của enemy
    int tmpRotation = 90;

    Uint32 spawnTime = 0;
    Uint32 lastMove = 0;

    int status = 1; // 1: Sống, 0: Chết
    int frame = 0;
};

#endif //TOWER_DEFENSE_PLAYENEMY_H

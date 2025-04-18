#include "Tower.h"

#ifndef TOWER_DEFENSE_PLAYTOWER_H
#define TOWER_DEFENSE_PLAYTOWER_H

struct playTower {
    Tower* tower;
    int x; // Tile x, từ 0 -> 19
    int y; // Tile y, từ 0 -> 10

    int level; // Level của tower, từ 0 -> 2
    int aimEnemy = -1; // Index của enemy đang nhắm, -1 nếu không nhắm
    Uint32 lastAttackTime = 0; // Thời gian cuối cùng tấn công
    int rotation = -90; // Góc quay của tower weapon, trong khoảng từ 0->359

};

#endif //TOWER_DEFENSE_PLAYTOWER_H

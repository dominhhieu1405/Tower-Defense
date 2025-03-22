#include "Tower.h"

#ifndef TOWER_DEFENSE_PLAYTOWER_H
#define TOWER_DEFENSE_PLAYTOWER_H

struct playTower {
    Tower* tower;
    int x; // Tile x, từ 0 -> 19
    int y; // Tile y, từ 0 -> 10

    int level; // Level của tower, từ 0 -> 2
    Uint32 lastAttackTime = 0; // Thời gian cuối cùng tấn công

};

#endif //TOWER_DEFENSE_PLAYTOWER_H

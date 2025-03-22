//
// Created by ADMIN on 19/03/2025.
//

#ifndef TOWER_DEFENSE_ENEMYMANAGER_H
#define TOWER_DEFENSE_ENEMYMANAGER_H


#include <iostream>
#include <vector>
#include <fstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <nlohmann/json.hpp>
#include "Enemy.h"

using json = nlohmann::json;

class EnemyManager {
public:
    std::vector<Enemy> enemies;

    void loadEnemy(SDL_Renderer* renderer, const std::string& filePath);
    void freeTextures();
};


#endif //TOWER_DEFENSE_ENEMYMANAGER_H

#ifndef TOWER_MANAGER_H
#define TOWER_MANAGER_H

#include <iostream>
#include <vector>
#include <fstream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <nlohmann/json.hpp>
#include "Tower.h"

using json = nlohmann::json;

class TowerManager {
public:
    std::vector<Tower> towers;

    void loadTowers(SDL_Renderer* renderer, const std::string& filePath);
    void freeTextures();
};

#endif // TOWER_MANAGER_H

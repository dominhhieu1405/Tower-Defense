//
// Created by ADMIN on 26/02/2025.
//

#ifndef TOWER_DEFENSE_STRUCT_H
#define TOWER_DEFENSE_STRUCT_H
#include <string>
#include <iostream>

struct Level {
    int id;
    std::string name;
    bool unlocked;
    int score;
    bool played;
};

struct Tile {
    SDL_Rect srcRect;
    SDL_Rect destRect;
};




#endif //TOWER_DEFENSE_STRUCT_H

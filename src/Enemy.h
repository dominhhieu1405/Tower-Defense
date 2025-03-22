//
// Created by ADMIN on 19/03/2025.
//

#ifndef TOWER_DEFENSE_ENEMY_H
#define TOWER_DEFENSE_ENEMY_H

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

struct EnemyLevel {
    int id;
    std::string name;
    int health;
    double speed;
    int reward;
    int frame;
};

struct Enemy {
    int id;
    std::string name;
    std::string imagePath;
    SDL_Texture *texture = nullptr;
    std::vector<EnemyLevel> levels;

    int frameWidth;
    int frameHeight;


    void loadTexture(SDL_Renderer* renderer) {
        SDL_Surface* surface = IMG_Load(imagePath.c_str());
        if (!surface) {
            std::cerr << "❌ Failed to load " << imagePath << ": " << IMG_GetError() << std::endl;
            return;
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "❌ Failed to create texture from " << imagePath << ": " << SDL_GetError() << std::endl;
        }
    }


    bool operator==(const Enemy& other) const {
        return id == other.id;
    }
};

#endif //TOWER_DEFENSE_ENEMY_H

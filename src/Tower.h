#ifndef TOWER_H
#define TOWER_H

#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// Cấu trúc cho Spritesheet
struct SpriteSheet {
    std::string path;
    int frameWidth;
    int frameHeight;
    int frameCount;
    SDL_Texture* texture = nullptr;

    void loadTexture(SDL_Renderer* renderer) {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            std::cerr << "❌ Failed to load " << path << ": " << IMG_GetError() << std::endl;
            return;
        }
        texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "❌ Failed to create texture from " << path << ": " << SDL_GetError() << std::endl;
        }
    }
};

// Cấu trúc cho từng cấp độ của tower
struct TowerLevel {
    int level;
    SpriteSheet projectile;
    SpriteSheet weapon;
    SpriteSheet impact;

    void loadTextures(SDL_Renderer* renderer) {
        projectile.loadTexture(renderer);
        weapon.loadTexture(renderer);
        impact.loadTexture(renderer);
    }
};

// Cấu trúc chính của Tower
struct Tower {
    int id;
    std::string name;
    std::string imagePath;
    SDL_Texture* texture = nullptr;

    int cost[3];          // Giá mua & nâng cấp (LV1 -> LV3)
    int damage[3];        // Sát thương theo từng level
    float attackSpeed[3]; // Tốc độ đánh theo từng level
    std::vector<TowerLevel> levels;

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
};

#endif // TOWER_H

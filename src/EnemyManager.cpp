//
// Created by ADMIN on 19/03/2025.
//

#include "EnemyManager.h"

void EnemyManager::loadEnemy(SDL_Renderer* renderer, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Error opening " << filePath << "!" << std::endl;
        return;
    }

    json enemyData;
    file >> enemyData;
    file.close();

    for (auto& t : enemyData["enemy"]) {
        Enemy enemy;
        enemy.id = t["id"];
        enemy.name = t["name"];
        enemy.imagePath = t["image"];
        enemy.frameWidth = t["frameWidth"];
        enemy.frameHeight = t["frameHeight"];

        // Load hình ảnh tower
        enemy.loadTexture(renderer);

        for (auto& l : t["levels"]) {
            EnemyLevel level;
            level.id = l["id"];
            level.name = l["name"];
            level.health = l["health"];
            level.speed = l["speed"];
            level.reward = l["reward"];
            level.frame = l["frame"];

            enemy.levels.push_back(level);
        }

        enemies.push_back(enemy);
    }

    std::cout << " - Loaded " << enemies.size() << " enemy successfully!" << std::endl;
}

void EnemyManager::freeTextures() {
    for (auto& Enemy : enemies) {
        if (Enemy.texture) {
            SDL_DestroyTexture(Enemy.texture);
            Enemy.texture = nullptr;
        }
        //for (auto& level : Enemy.levels) {

        //}
    }
    std::cout << " -  Freed tower textures!" << std::endl;
}
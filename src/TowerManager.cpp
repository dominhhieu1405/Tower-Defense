#include "TowerManager.h"

void TowerManager::loadTowers(SDL_Renderer* renderer, const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        std::cerr << "Error opening " << filePath << "!" << std::endl;
        return;
    }

    json towerData;
    file >> towerData;
    file.close();

    for (auto& t : towerData["towers"]) {
        Tower tower;
        tower.id = t["id"];
        tower.name = t["name"];
        tower.imagePath = t["image"];

        for (int i = 0; i < 3; i++) {
            tower.cost[i] = t["cost"][i];
            tower.damage[i] = t["damage"][i];
            tower.attackSpeed[i] = t["attack_speed"][i];
            tower.range[i] = t["range"][i];
        }

        // Load hình ảnh tower
        tower.loadTexture(renderer);

        for (auto& l : t["levels"]) {
            TowerLevel level;
            level.level = l["level"];

            // Load spritesheets
            level.projectile.path = l["projectile"]["path"];
            level.projectile.frameWidth = l["projectile"]["frame_width"];
            level.projectile.frameHeight = l["projectile"]["frame_height"];
            level.projectile.frameCount = l["projectile"]["frame_count"];

            level.weapon.path = l["weapon"]["path"];
            level.weapon.frameWidth = l["weapon"]["frame_width"];
            level.weapon.frameHeight = l["weapon"]["frame_height"];
            level.weapon.frameCount = l["weapon"]["frame_count"];

            level.impact.path = l["impact"]["path"];
            level.impact.frameWidth = l["impact"]["frame_width"];
            level.impact.frameHeight = l["impact"]["frame_height"];
            level.impact.frameCount = l["impact"]["frame_count"];

            level.loadTextures(renderer);
            tower.levels.push_back(level);
        }

        towers.push_back(tower);
    }

    std::cout << "- Loaded " << towers.size() << " towers successfully!" << std::endl;
}

void TowerManager::freeTextures() {
    for (auto& tower : towers) {
        if (tower.texture) {
            SDL_DestroyTexture(tower.texture);
            tower.texture = nullptr;
        }
        for (auto& level : tower.levels) {
            if (level.projectile.texture) {
                SDL_DestroyTexture(level.projectile.texture);
                level.projectile.texture = nullptr;
            }
            if (level.weapon.texture) {
                SDL_DestroyTexture(level.weapon.texture);
                level.weapon.texture = nullptr;
            }
            if (level.impact.texture) {
                SDL_DestroyTexture(level.impact.texture);
                level.impact.texture = nullptr;
            }
        }
    }
    std::cout << "- Freed tower textures!" << std::endl;
}

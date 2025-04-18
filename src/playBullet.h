//
// Created by ADMIN on 14/04/2025.
//

#ifndef TOWER_DEFENSE_PLAYBULLET_H
#define TOWER_DEFENSE_PLAYBULLET_H


#include <SDL2/SDL.h>

struct Bullet {
    int towerIndex;     // Chỉ số của tower bắn đạn (hoặc có thể lưu trực tiếp pointer)
    int enemyIndex;     // Chỉ số enemy mục tiêu
    float startX;       // Tọa độ bắt đầu (center của tower)
    float startY;
    float targetX;      // Tọa độ mục tiêu (center của enemy)
    float targetY;
    Uint32 spawnTime;   // Thời điểm viên đạn được tạo
    Uint32 duration;    // Thời gian bay (100ms)
    // Thông tin để render hoạt ảnh đạn từ SpriteSheet impact:
    SDL_Texture* texture;
    int frameWidth;     // Chiều rộng mỗi frame
    int frameHeight;    // Chiều cao mỗi frame
    int frameCount;     // Số frame của hoạt ảnh
    int rotation;      // Góc quay của viên đạn (nếu cần)
};

#endif //TOWER_DEFENSE_PLAYBULLET_H

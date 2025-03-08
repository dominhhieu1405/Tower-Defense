#ifndef PLAY_H
#define PLAY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <string>
#include "Game.h"

class Play {
public:
    Play(SDL_Renderer* renderer, bool* isRunning, Game* game);
    ~Play();

    void loadMap(int levelID);
    void render();
    void update();
    void handleEvent(SDL_Event& event);

private:
    SDL_Renderer* renderer;
    bool* isRunning;
    Game* game;
    SDL_Texture* tilesetTexture;
    std::vector<std::vector<int>> mapData;



    const int TILE_SIZE = 64;
    const int MAP_WIDTH = 20;
    const int MAP_HEIGHT = 11;
    const int TILESET_COLS = 16; // 1024 / 64 = 16 tiles per row

    int highlightTileX = -1;
    int highlightTileY = -1;
    bool highlightValid = false;
    //std::vector<int> allowTiles = {18, 33, 35, 38, 44, 45, 50, 60, 61, 118, 155, 156, 202, 204, 218, 220, 234, 236}; //Tăng 1 đơn vị
    std::vector<int> allowTiles = {19, 34, 36, 39, 45, 46, 51, 61, 62, 119, 156, 157, 203, 205, 219, 221, 235, 237}; //Tăng 1 đơn vị

    Uint32 startTick = 0;
    Uint32 lastFrameTime = 0;
    int currentFrame = 0;

    TTF_Font* gameFont = nullptr;

    int money = 200;
    int lives = 5;

    // Kéo thả tower
    bool isDragging = false;
    Tower* draggedTower = nullptr;
    int mouseX, mouseY;

    SDL_Texture* loadTexture(const std::string& path);
    void parseMapJson(const std::string& filePath);

};

#endif

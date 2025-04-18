#ifndef PLAY_H
#define PLAY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <string>
#include <unordered_set>
#include "Game.h"
#include "playTower.h"
#include "playEnemy.h"
#include "playBullet.h"



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
    SDL_Texture* buttonsTexture;
    std::vector<std::vector<int>> mapData;

    std::vector<playTower> towers;
    std::vector<playEnemy> enemies;
    std::vector<Bullet> bullets;
    std::unordered_set<int> spawned = {};
    playTower* selectedTower = nullptr;

    int currentRange = 0;
    SDL_Texture *rangeTexture = nullptr;

    nlohmann::json levelData;

    const int TILE_SIZE = 64;
    const int MAP_WIDTH = 20;
    const int MAP_HEIGHT = 11;
    const int TILESET_COLS = 16; // 1024 / 64 = 16 tiles per row

    int highlightTileX = -1;
    int highlightTileY = -1;
    bool highlightValid = false;
    //std::vector<int> allowTiles = {18, 33, 35, 38, 44, 45, 50, 60, 61, 118, 155, 156, 202, 204, 218, 220, 234, 236}; //Tăng 1 đơn vị
    std::vector<int> allowTiles = {19, 34, 36, 39, 45, 46, 51, 61, 62, 119, 156, 157, 203, 205, 219, 221, 235, 237}; //Tăng 1 đơn vị

    bool playing = true;
    Uint32 stopTick = 0;
    Uint32 startTick = 0;
    Uint32 timeTick = 0;
    Uint32 lastFrameTime = 0;
    Uint32 endTick = 0;

    int currentFrame = 0;
    int hoverButton = -1;

    SDL_Texture* logoTexture;
    TTF_Font* gameFont = nullptr;
    Mix_Chunk* placeSound;
    Mix_Chunk* clickSound;
    Mix_Chunk* tempSound = nullptr;
    int soundStep = 0;

    int money = 500;
    int lives = 5;

    //
    int totalEnemies = 0;
    std::unordered_set<int> killedEnemies;
    std::unordered_set<int> aliveEnemies;
    std::unordered_set<int> failedEnemies;


    // Kéo thả tower
    bool isDragging = false;
    Tower* draggedTower = nullptr;
    int mouseX, mouseY;

    SDL_Texture* loadTexture(const std::string& path);
    void parseMapJson(const std::string& filePath);

    void spawnEnemy();
    void moveEnemies();
    void renderEnemies();

    void movePlayTowers();
    void attackEnemies();
    void renderPlayTowers();
    void updateBullets();
    void renderBullets();

    void renderDraggingTower();

    void updateScore();

    void createRangeCircle(int radius);
};

#endif

#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "TowerManager.h"
#include "EnemyManager.h"


class Menu;
class LevelSelect;
class Play;
class Leaderboard;

enum GameState {
    MENU,
    LEVEL_SELECT,
    PLAY,
    LEADERBOARD
};

class Game {

public:
    Game();
    ~Game();
    Mix_Music* bgm;
    GameState currentState;
    int selectedLevel = -1;

    TowerManager towerManager;
    EnemyManager enemyManager;

    bool init(const char* title, int width, int height);

    void run();
    void handleEvents();
    void update();
    void render();
    void cleanup();

private:

    Menu* menu;
    LevelSelect* levelSelect;
    Play* play = nullptr;
    Leaderboard* leaderboard;

    SDL_Window* window;
    SDL_Renderer* renderer = nullptr;
    bool isRunning;
};

#endif // GAME_H

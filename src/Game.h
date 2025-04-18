#ifndef GAME_H
#define GAME_H

#include <cstdlib>
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
    std::string dataPath = "assets/data/data.json";
    std::string levelFile = "assets/data/levels.json";

    TowerManager towerManager;
    EnemyManager enemyManager;

    bool init(const char* title, int width, int height);

    void run();
    void handleEvents();
    void update();
    void render();
    void openURL(const std::string& url);
    void cleanup();

    Play* play = nullptr;
    Menu* menu = nullptr;
    LevelSelect* levelSelect;
    Leaderboard* leaderboard;
private:


    SDL_Window* window;
    SDL_Renderer* renderer = nullptr;
    bool isRunning;
};

#endif // GAME_H

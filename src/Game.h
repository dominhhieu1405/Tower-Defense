#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "Menu.h"

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

    bool init(const char* title, int width, int height);

    void run();
    void handleEvents();
    void update();
    void render();
    void cleanup();

private:

    Menu* menu;
    SDL_Window* window;
    SDL_Renderer* renderer = nullptr;
    bool isRunning;
    GameState currentState;
};

#endif // GAME_H

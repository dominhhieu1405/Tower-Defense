#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include "Game.h"
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class Leaderboard {
public:
    Leaderboard(SDL_Renderer* renderer, bool* isRunning, Game* game);
    ~Leaderboard();

    void handleEvents(SDL_Event& event);
    void render();


private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    TTF_Font* font2;
    std::vector<std::string> ranks;

    int hoveredButton = -1;


    SDL_Texture* backgroundTexture;
    SDL_Texture* buttonTexture;
    SDL_Texture* tableTexture;
    Mix_Chunk* clickSound;


    bool* isRunning;
    Game* game; // Truy cập `currentState`

    void loadScores();
};

#endif

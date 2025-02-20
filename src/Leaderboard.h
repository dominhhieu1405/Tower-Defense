#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Game.h"
#include <vector>

class Leaderboard {
public:
    Leaderboard(SDL_Renderer* renderer);
    ~Leaderboard();

    void handleEvents(SDL_Event& event, GameState& currentState);
    void render();
    void saveScore(int newScore);

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::vector<int> scores;

    void loadScores();
};

#endif

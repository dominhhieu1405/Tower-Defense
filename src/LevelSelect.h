#ifndef LEVELSELECT_H
#define LEVELSELECT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Game.h"
#include <string>

class LevelSelect {
public:
    LevelSelect(SDL_Renderer* renderer);
    ~LevelSelect();

    void handleEvents(SDL_Event& event, GameState& currentState);
    void render();

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    int selectedLevel;
};

#endif

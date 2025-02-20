#ifndef PLAY_H
#define PLAY_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Game.h"

class Play {
public:
    Play(SDL_Renderer* renderer);
    ~Play();

    void handleEvents(SDL_Event& event, GameState& currentState);
    void update();
    void render();

private:
    SDL_Renderer* renderer;
    TTF_Font* font;
    bool isPlaying;
};

#endif

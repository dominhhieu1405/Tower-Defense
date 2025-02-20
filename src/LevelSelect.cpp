#include "LevelSelect.h"
#include <SDL2/SDL_ttf.h>

LevelSelect::LevelSelect(SDL_Renderer* renderer) : renderer(renderer), selectedLevel(0) {
    font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    if (!font) {
        SDL_Log("Không thể tải font! Lỗi: %s", TTF_GetError());
    }
}

LevelSelect::~LevelSelect() {
    TTF_CloseFont(font);
}

void LevelSelect::handleEvents(SDL_Event& event, GameState& currentState) {
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_LEFT:
                selectedLevel = (selectedLevel - 1 + 5) % 5;
                break;
            case SDLK_RIGHT:
                selectedLevel = (selectedLevel + 1) % 5;
                break;
            case SDLK_RETURN:
                currentState = PLAY;
                break;
            case SDLK_ESCAPE:
                currentState = MENU;
                break;
        }
    }
}

void LevelSelect::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    for (int i = 0; i < 5; i++) {
        SDL_Color color = (i == selectedLevel) ? yellow : white;
        std::string levelText = "Level " + std::to_string(i + 1);

        SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, levelText.c_str(), color);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {200 + i * 100, 250, textSurface->w, textSurface->h};

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    SDL_RenderPresent(renderer);
}

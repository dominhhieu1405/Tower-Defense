#ifndef LEVELSELECT_H
#define LEVELSELECT_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <vector>
#include <string>
#include "Game.h"

struct Level {
    int id;
    std::string name;
    bool unlocked;
    int score;
    bool played;
};

class LevelSelect {
public:
    LevelSelect(SDL_Renderer* renderer, bool* isRunning, Game* game);
    ~LevelSelect();

    void loadLevels(const char* filename);
    void render();
    void handleEvents(SDL_Event& event);

    SDL_Rect backButton;
    int hoveredButton = -1;
private:
    SDL_Renderer* renderer;
    SDL_Texture* backgroundTexture;
    SDL_Texture* logoTexture;
    SDL_Texture* buttonTexture;
    SDL_Texture* starOnTexture;
    SDL_Texture* starOffTexture;
    TTF_Font* font;
    Mix_Chunk* clickSound;

    bool* isRunning;
    Game* game; // Truy cập `currentState`
    std::vector<Level> levels;

    SDL_Rect levelButtons[9];

    void renderText(const char* text, int x, int y);
    void renderStars(int score, int x, int y);
};

#endif // LEVELSELECT_H

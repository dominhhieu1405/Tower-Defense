#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string>

class Game;

class NameInput {
public:
    NameInput(SDL_Renderer* renderer, bool* isRunning, Game* game, const std::string& dataPath);
    ~NameInput();
    void render();
    void handleEvent(SDL_Event& e);

private:
    SDL_Renderer* renderer;
    bool* isRunning;
    Game* game;


    SDL_Texture* backgroundTexture;
    SDL_Texture* logoTexture;
    SDL_Texture* inputTexture;
    Mix_Chunk* clickSound;

    TTF_Font* font;
    TTF_Font* font2;
    std::string inputText;
    SDL_Texture* textTexture;
    SDL_Rect inputBox;
    std::string dataPath;

    void saveName();
    void updateTexture();
};

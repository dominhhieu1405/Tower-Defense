#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <string>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int PLAYER_WIDTH = 100;
const int PLAYER_HEIGHT = 20;
const int ITEM_WIDTH = 30;
const int ITEM_HEIGHT = 30;
const int ITEM_SPEED = 5;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

struct Item {
    int x, y;
};

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() == -1) {
        std::cerr << "TTF could not initialize! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }
    window = SDL_CreateWindow("Hứng vật phẩm", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    font = TTF_OpenFont("font.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }
    return true;
}

void closeSDL() {
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

void renderText(const std::string& text, int x, int y) {
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect renderQuad = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

int main(int argc, char* args[]) {
    if (!initSDL()) return -1;

    int playerX = (SCREEN_WIDTH - PLAYER_WIDTH) / 2;
    int playerY = SCREEN_HEIGHT - PLAYER_HEIGHT - 10;
    std::vector<Item> items;
    int score = 0;
    bool quit = false;
    SDL_Event e;

    srand(static_cast<unsigned>(time(0)));

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_LEFT && playerX > 0) {
                    playerX -= 20;
                } else if (e.key.keysym.sym == SDLK_RIGHT && playerX < SCREEN_WIDTH - PLAYER_WIDTH) {
                    playerX += 20;
                }
            }
        }

        if (rand() % 30 == 0) {
            items.push_back({rand() % (SCREEN_WIDTH - ITEM_WIDTH), 0});
        }

        for (auto& item : items) {
            item.y += ITEM_SPEED;
        }

        items.erase(std::remove_if(items.begin(), items.end(), [&](Item& item) {
            if (item.y > SCREEN_HEIGHT) return true;
            if (item.y + ITEM_HEIGHT >= playerY && item.x + ITEM_WIDTH >= playerX && item.x <= playerX + PLAYER_WIDTH) {
                score++;
                return true;
            }
            return false;
        }), items.end());

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect playerRect = {playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT};
        SDL_RenderFillRect(renderer, &playerRect);

        for (const auto& item : items) {
            SDL_Rect itemRect = {item.x, item.y, ITEM_WIDTH, ITEM_HEIGHT};
            SDL_RenderFillRect(renderer, &itemRect);
        }

        renderText("Score: " + std::to_string(score), 10, 10);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    closeSDL();
    return 0;
}

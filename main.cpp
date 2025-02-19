#define SDL_MAIN_HANDLED  // Bỏ qua hàm main mặc định của SDL để tránh lỗi khi biên dịch trên một số nền tảng.

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <vector>

using json = nlohmann::json;

const int TILE_SIZE = 64;
const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 11;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
SDL_Texture* tilesetTexture = nullptr;

std::vector<int> mapData;

bool initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    window = SDL_CreateWindow("Tilemap", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MAP_WIDTH * TILE_SIZE, MAP_HEIGHT * TILE_SIZE, SDL_WINDOW_SHOWN);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    return renderer != nullptr;
}

SDL_Texture* loadTexture(const char* path) {
    SDL_Surface* loadedSurface = IMG_Load(path);
    if (!loadedSurface) return nullptr;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    SDL_FreeSurface(loadedSurface);
    return texture;
}

void loadMap(const std::string& filename) {
    std::ifstream file(filename);
    json j;
    file >> j;

    mapData = j["layers"][0]["data"].get<std::vector<int>>();
}

void renderMap() {
    for (int row = 0; row < MAP_HEIGHT; row++) {
        for (int col = 0; col < MAP_WIDTH; col++) {
            int tileIndex = mapData[row * MAP_WIDTH + col] - 1; // Trừ 1 vì firstgid = 1

            if (tileIndex < 0) continue; // Bỏ qua ô trống

            SDL_Rect srcRect = { (tileIndex % 16) * TILE_SIZE, (tileIndex / 16) * TILE_SIZE, TILE_SIZE, TILE_SIZE };
            SDL_Rect dstRect = { col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE };

            SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &dstRect);
        }
    }
}

void cleanUp() {
    SDL_DestroyTexture(tilesetTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* args[]) {
    if (!initSDL()) return -1;

    tilesetTexture = loadTexture("./assets/images/Tileset/tileset.png");
    if (!tilesetTexture) return -1;

    loadMap("./assets/maps/1.json");

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }

        SDL_RenderClear(renderer);
        renderMap();
        SDL_RenderPresent(renderer);
    }

    cleanUp();
    return 0;
}
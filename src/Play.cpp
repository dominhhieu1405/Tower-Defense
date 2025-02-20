#include "Play.h"
#include <SDL2/SDL_ttf.h>

Play::Play(SDL_Renderer* renderer) : renderer(renderer), isPlaying(true) {
    font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    if (!font) {
        SDL_Log("Không thể tải font! Lỗi: %s", TTF_GetError());
    }
}

Play::~Play() {
    TTF_CloseFont(font);
}

void Play::handleEvents(SDL_Event& event, GameState& currentState) {
    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_ESCAPE) {
            currentState = MENU;  // Quay lại menu khi nhấn ESC
        }
    }
}

void Play::update() {
    // Cập nhật logic game: kẻ địch di chuyển, tháp bắn, kiểm tra chiến thắng/thua
}

void Play::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Hiển thị chữ "Đang chơi..."
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, "Đang chơi...", white);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {300, 250, textSurface->w, textSurface->h};

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    SDL_RenderPresent(renderer);
}

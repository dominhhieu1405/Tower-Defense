#include "Leaderboard.h"
#include <SDL2/SDL_ttf.h>
#include <fstream>
#include <vector>
#include <algorithm>

Leaderboard::Leaderboard(SDL_Renderer* renderer) : renderer(renderer) {
    font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    if (!font) {
        SDL_Log("Không thể tải font! Lỗi: %s", TTF_GetError());
    }
    loadScores();
}

Leaderboard::~Leaderboard() {
    TTF_CloseFont(font);
}

void Leaderboard::handleEvents(SDL_Event& event, GameState& currentState) {
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
        currentState = MENU;  // Quay lại menu khi nhấn ESC
    }
}

void Leaderboard::render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Color white = {255, 255, 255, 255};
    SDL_Color yellow = {255, 255, 0, 255};

    // Tiêu đề
    SDL_Surface* titleSurface = TTF_RenderUTF8_Solid(font, "Bảng Xếp Hạng", yellow);
    SDL_Texture* titleTexture = SDL_CreateTextureFromSurface(renderer, titleSurface);
    SDL_Rect titleRect = {300, 50, titleSurface->w, titleSurface->h};
    SDL_RenderCopy(renderer, titleTexture, NULL, &titleRect);
    SDL_FreeSurface(titleSurface);
    SDL_DestroyTexture(titleTexture);

    // Hiển thị danh sách điểm
    for (size_t i = 0; i < scores.size() && i < 5; i++) {
        std::string scoreText = std::to_string(i + 1) + ". " + std::to_string(scores[i]);
        SDL_Surface* textSurface = TTF_RenderUTF8_Solid(font, scoreText.c_str(), white);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {300, 150 + (int)i * 50, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    SDL_RenderPresent(renderer);
}

void Leaderboard::loadScores() {
    scores.clear();
    std::ifstream file("scores.txt");
    int score;
    while (file >> score) {
        scores.push_back(score);
    }
    file.close();

    std::sort(scores.rbegin(), scores.rend());  // Sắp xếp giảm dần
}

void Leaderboard::saveScore(int newScore) {
    scores.push_back(newScore);
    std::sort(scores.rbegin(), scores.rend());

    std::ofstream file("scores.txt");
    for (size_t i = 0; i < scores.size() && i < 5; i++) {
        file << scores[i] << "\n";
    }
    file.close();
}

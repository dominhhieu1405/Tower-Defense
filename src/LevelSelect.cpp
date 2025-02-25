#include "LevelSelect.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 80
#define BUTTON_SPACING 20

LevelSelect::LevelSelect(SDL_Renderer* renderer, bool* isRunning, Game* game)
        : renderer(renderer), game(game), isRunning(isRunning), backgroundTexture(nullptr), logoTexture(nullptr),
          buttonTexture(nullptr), starOnTexture(nullptr), starOffTexture(nullptr), font(nullptr), clickSound(nullptr) {

    backgroundTexture = IMG_LoadTexture(renderer, "assets/images/background.png");
    logoTexture = IMG_LoadTexture(renderer, "assets/images/logo.png");
    buttonTexture = IMG_LoadTexture(renderer, "assets/images/button.png");
    starOnTexture = IMG_LoadTexture(renderer, "assets/images/star-on.png");
    starOffTexture = IMG_LoadTexture(renderer, "assets/images/star-off.png");
    font = TTF_OpenFont("assets/fonts/wood.ttf", 32);
    clickSound = Mix_LoadWAV("assets/audios/click.wav");

    loadLevels("assets/data/levels.json");

    int startX = (1200 - (3 * BUTTON_WIDTH + 2 * BUTTON_SPACING)) / 2;
    int startY = 250 + 50;  // Dưới logo

    for (int i = 0; i < 9; i++) {
        int row = i / 3;
        int col = i % 3;
        levelButtons[i] = { startX + col * (BUTTON_WIDTH + BUTTON_SPACING), startY + row * (BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT };
    }

    backButton = { (1200 - BUTTON_WIDTH) / 2, startY + 3 * (BUTTON_HEIGHT + BUTTON_SPACING), BUTTON_WIDTH, BUTTON_HEIGHT };
}

LevelSelect::~LevelSelect() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(logoTexture);
    SDL_DestroyTexture(buttonTexture);
    SDL_DestroyTexture(starOnTexture);
    SDL_DestroyTexture(starOffTexture);
    if (font) TTF_CloseFont(font);
    Mix_FreeChunk(clickSound);
}

void LevelSelect::loadLevels(const char* filename) {
    std::ifstream file(filename);
    if (!file) {
        SDL_Log("Failed to load levels data!");
        return;
    }

    json levelData;
    file >> levelData;

    for (const auto& item : levelData) {
        levels.push_back({
                                 item["id"],
                                 item["name"],
                                 item["unlocked"],
                                 item["score"],
                                 item["played"]
                         });
    }
}

void LevelSelect::render() {
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    SDL_Rect logoRect = { (1200 - 500) / 2, 35, 500, 250 };
    SDL_RenderCopy(renderer, logoTexture, NULL, &logoRect);

    for (int i = 0; i < levels.size(); i++) {
        Level& level = levels[i];

        if (!level.unlocked) {
            SDL_SetTextureColorMod(buttonTexture, 100, 100, 100);  // Làm tối màu
        } else {
            if (hoveredButton == i) {
                SDL_SetTextureColorMod(buttonTexture, 200, 200, 180);  // Làm sáng màu
            } else {
                SDL_SetTextureColorMod(buttonTexture, 255, 255, 255);
            }
        }


        SDL_RenderCopy(renderer, buttonTexture, NULL, &levelButtons[i]);
        if (level.unlocked && level.played) {
            renderText(level.name.c_str(), levelButtons[i].x + BUTTON_WIDTH / 2, levelButtons[i].y + BUTTON_HEIGHT / 2 - 20);
            renderStars(level.score, levelButtons[i].x + 60, levelButtons[i].y + 45);
        } else {
            renderText(level.name.c_str(), levelButtons[i].x + BUTTON_WIDTH / 2, levelButtons[i].y + BUTTON_HEIGHT / 2 - 5);
        }
    }

    if (hoveredButton == 99) {
        SDL_SetTextureColorMod(buttonTexture, 200, 200, 180);  // Làm sáng màu
    } else {
        SDL_SetTextureColorMod(buttonTexture, 255, 255, 255);
    }
    SDL_RenderCopy(renderer, buttonTexture, NULL, &backButton);
    renderText("QUAY LẠI", backButton.x + BUTTON_WIDTH / 2, backButton.y + BUTTON_HEIGHT / 2 - 5);

    SDL_RenderPresent(renderer);
}

void LevelSelect::renderText(const char* text, int x, int y) {
    if (!font) return;

    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text, textColor);
    if (!textSurface) return;

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { x - textSurface->w / 2, y - textSurface->h / 2, textSurface->w, textSurface->h };

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void LevelSelect::renderStars(int score, int x, int y) {
    SDL_Rect starRect = { x, y, 24, 24 };

    for (int i = 0; i < 3; i++) {
        if (i < score) {
            SDL_RenderCopy(renderer, starOnTexture, NULL, &starRect);
        } else {
            SDL_RenderCopy(renderer, starOffTexture, NULL, &starRect);
        }
        starRect.x += 30;
    }
}



void LevelSelect::handleEvents(SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;
        hoveredButton = -1;

        for (int i = 0; i < levels.size(); i++) {
            if (mouseX >= levelButtons[i].x && mouseX <= levelButtons[i].x + BUTTON_WIDTH &&
                mouseY >= levelButtons[i].y && mouseY <= levelButtons[i].y + BUTTON_HEIGHT &&
                levels[i].unlocked) {
                hoveredButton = i;
                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_Log("Clicked level %d", levels[i].id);
                    Mix_PlayChannel(-1, clickSound, 0);
                    game->selectedLevel = levels[i].id;
                    game->currentState = PLAY;
                }
            }
        }

        if (mouseX >= backButton.x && mouseX <= backButton.x + BUTTON_WIDTH &&
            mouseY >= backButton.y && mouseY <= backButton.y + BUTTON_HEIGHT) {
            hoveredButton = 99;
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                Mix_PlayChannel(-1, clickSound, 0);
                SDL_Log("Returning to menu...");
                game->currentState = MENU;
            }
        }
    }

}

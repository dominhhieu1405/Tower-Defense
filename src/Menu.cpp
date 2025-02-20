#include "Menu.h"
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

// Định nghĩa khoảng cách giữa các nút.

#define BUTTON_SPACING 20

Menu::Menu(SDL_Renderer* renderer, bool* isRunning) : renderer(renderer), backgroundTexture(nullptr), logoTexture(nullptr), buttonTexture(nullptr), font(nullptr), bgm(nullptr), isRunning(isRunning) {
    // Load hình nền
    backgroundTexture = IMG_LoadTexture(renderer, "assets/images/background.png");
    if (!backgroundTexture) {
        SDL_Log("Failed to load background: %s", SDL_GetError());
    }

    // Load logo
    logoTexture = IMG_LoadTexture(renderer, "assets/images/logo.png");
    if (!logoTexture) {
        SDL_Log("Failed to load logo: %s", SDL_GetError());
    }

    // Load ảnh nút
    buttonTexture = IMG_LoadTexture(renderer, "assets/images/button.png");
    if (!buttonTexture) {
        SDL_Log("Failed to load button: %s", SDL_GetError());
    }

    // Load font chữ
    font = TTF_OpenFont("assets/fonts/wood.ttf", 38);
    if (!font) {
        SDL_Log("Failed to load font: %s", TTF_GetError());
    }

    const int buttonWidth = 250;
    const int buttonHeight = 80;
    const int buttonSpacing = 20;
    int buttonStartY = 35 + 250 + 28; // Căn dưới logo

    // Gán giá trị chính xác cho `buttons[]`
    for (int i = 0; i < 4; i++) {
        buttons[i] = {
                (1200 - buttonWidth) / 2,
                buttonStartY + i * (buttonHeight + buttonSpacing),
                buttonWidth,
                buttonHeight
        };
    }


    clickSound = Mix_LoadWAV("assets/audios/click.wav");
    if (!clickSound) {
        SDL_Log("Failed to load click sound: %s", Mix_GetError());
    }
}

Menu::~Menu() {
    // Giải phóng tài nguyên
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(logoTexture);
    SDL_DestroyTexture(buttonTexture);
    if (font) TTF_CloseFont(font);
    //if (bgm) Mix_FreeMusic(bgm);
    Mix_FreeChunk(clickSound);
    Mix_CloseAudio();
}

void Menu::render() {

    int screenWidth, screenHeight;
    SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

    // Vẽ nền
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    // Hiển thị logo (căn giữa ngang, lệch lên trên)
    const int logoWidth = 500;
    const int logoHeight = 250;
    SDL_Rect logoRect = {
            (1200 - logoWidth) / 2,  // Giữa theo chiều ngang
            35,  // Cách mép trên 35px
            logoWidth,
            logoHeight
    };
    SDL_RenderCopy(renderer, logoTexture, NULL, &logoRect);

    // Kích thước nút
    const int buttonWidth = 250;
    const int buttonHeight = 80;
    const int buttonSpacing = 20;

    // Tọa độ nút đầu tiên (dưới logo, cách 35px)
    int buttonStartY = 35 + logoHeight + 28;

    // Danh sách chữ trên các nút
    const char* buttonTexts[] = { "CHƠI TIẾP", "BẢN ĐỒ", "XẾP HẠNG", "THOÁT" };

    for (int i = 0; i < 4; i++) {


        if (i == hoveredButton) {
            SDL_SetTextureColorMod(buttonTexture, 180, 180, 180);  // Giảm sáng xuống 180/255
        } else {
            SDL_SetTextureColorMod(buttonTexture, 255, 255, 255);  // Trả về màu gốc
        }

        // Vẽ nút
        SDL_RenderCopy(renderer, buttonTexture, NULL, &buttons[i]);

        // Hiển thị chữ trên nút (căn giữa)
        renderText(buttonTexts[i], buttons[i].x + buttonWidth / 2, buttons[i].y + buttonHeight / 2);
    }

    SDL_RenderPresent(renderer);
}


void Menu::renderText(const char* text, int x, int y) {
    if (!font) return;

    SDL_Color textColor = { 255, 255, 255, 255 }; // Màu trắng
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text, textColor);
    if (!textSurface) return;

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        SDL_FreeSurface(textSurface);
        return;
    }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_Rect textRect = { x - textWidth / 2, y - textHeight / 2 - 5, textWidth, textHeight };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
void Menu::handleEvents(SDL_Event& event) {
    if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;
        hoveredButton = -1;

        for (int i = 0; i < 4; i++) {
            if (mouseX >= buttons[i].x && mouseX <= buttons[i].x + buttons[i].w &&
                mouseY >= buttons[i].y && mouseY <= buttons[i].y + buttons[i].h) {
                hoveredButton = i;


                if (event.type == SDL_MOUSEBUTTONDOWN) {
                    SDL_Log("Clicked button %d", i);


                    // Xử lý click vào từng nút nếu cần
                    if (clickSound) {
                        Mix_PlayChannel(-1, clickSound, 0);
                    }
                    if (i == 3) {
                        // Thoát game
                        SDL_Log("Exiting game...");
                        *isRunning = false;  // Thoát game
                    }
                }
            }
        }
    }
}


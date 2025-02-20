#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

class Menu {
public:
    SDL_Rect buttons[4];
    int hoveredButton = -1; // -1 nghĩa là không có nút nào đang hover
    explicit Menu(SDL_Renderer* renderer, bool* isRunning);
    ~Menu();

    void render(); // Vẽ màn hình menu
    void handleEvents(SDL_Event &event);

private:
    SDL_Renderer* renderer;

    // Texture
    SDL_Texture* backgroundTexture;
    SDL_Texture* logoTexture;
    SDL_Texture* buttonTexture;
    Mix_Chunk* clickSound;

    // Font chữ
    TTF_Font* font;

    // Nhạc nền
    Mix_Music* bgm;
    bool* isRunning;

    void renderText(const char* text, int x, int y);

};

#endif // MENU_H

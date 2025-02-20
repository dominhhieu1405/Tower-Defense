#include "Game.h"
#include "Menu.h"
#include "LevelSelect.h"
#include "Play.h"
#include "Leaderboard.h"

Game::Game() : window(nullptr), renderer(nullptr), isRunning(true), currentState(MENU), menu(nullptr) {}

Game::~Game() {
    delete menu;
    cleanup();
}

bool Game::init(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
        return false;
    }

    if (!IMG_Init(IMG_INIT_PNG)) {
        SDL_Log("SDL_image could not initialize! IMG_Error: %s", IMG_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
        SDL_Log("SDL_ttf could not initialize! TTF_Error: %s", TTF_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("SDL_mixer could not initialize! Mix_Error: %s", Mix_GetError());
        return false;
    }
    // Khởi tạo SDL_mixer với tần số 44100Hz, 2 kênh (stereo), buffer 2048
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_Log("Mix_OpenAudio failed: %s", Mix_GetError());
        return false;
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
        return false;
    }
    // Phát nhạc nền
    bgm = Mix_LoadMUS("assets/audios/background.mp3");

    if (!bgm) {
        SDL_Log("Failed to load background music: %s", Mix_GetError());
    } else {
        Mix_VolumeMusic( MIX_MAX_VOLUME);  // Đặt âm lượng tối đa
        Mix_PlayMusic(bgm, -1);  // Phát nhạc lặp vô hạn
    }

    // Khởi tạo menu sau khi renderer đã có
    menu = new Menu(renderer, &isRunning);
    return true;
}

void Game::run() {
    //Menu menu(renderer);  // Tạo menu 1 lần duy nhất
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }

            menu->handleEvents(event);  // Gọi hàm xử lý sự kiện cho menu
            render();
        }

        update();
        render();
        SDL_Delay(16);  // Giữ ~60 FPS
    }
}


void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            int x = event.button.x;
            int y = event.button.y;

            // Xử lý click chuột trên menu
            if (currentState == MENU) {
//                if (/* Kiểm tra nếu bấm vào nút "Chơi tiếp" */) {
//                    currentState = PLAY;
//                } else if (/* Kiểm tra nếu bấm vào "Bản đồ" */) {
//                    currentState = LEVEL_SELECT;
//                } else if (/* Kiểm tra nếu bấm vào "Bảng xếp hạng" */) {
//                    currentState = LEADERBOARD;
//                } else if (/* Kiểm tra nếu bấm vào "Thoát" */) {
//                    isRunning = false;
//                }
            }
        }
    }
}

void Game::update() {
    // Chưa cần xử lý gì nhiều, chỉ chuyển màn
}

void Game::render() {
    SDL_RenderClear(renderer);

    if (currentState == MENU) {
        //Menu menu(renderer);
        menu->render();
    } else if (currentState == LEVEL_SELECT) {
        LevelSelect levelSelect(renderer);
        levelSelect.render();
    } else if (currentState == PLAY) {
        Play play(renderer);
        play.render();
    } else if (currentState == LEADERBOARD) {
        Leaderboard leaderboard(renderer);
        leaderboard.render();
    }

    SDL_RenderPresent(renderer);
}

void Game::cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

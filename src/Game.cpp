#include "Game.h"
#include "Menu.h"
#include "LevelSelect.h"
#include "Play.h"
#include "Leaderboard.h"
#include <cstdlib>



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
    SDL_Surface* icon = IMG_Load("assets/images/favicon.png");
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    } else {
        SDL_Log("Failed to load icon: %s", SDL_GetError());
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

    towerManager.loadTowers(renderer, "assets/data/towers.json");
    enemyManager.loadEnemy(renderer, "assets/data/enemies.json");
    // Khởi tạo menu sau khi renderer đã có

    menu = new Menu(renderer, &isRunning, this);
    levelSelect = new LevelSelect(renderer, &isRunning, this);


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

            handleEvents();
            if (currentState == MENU) {
                menu->handleEvents(event);
            } else if (currentState == LEVEL_SELECT) {
                levelSelect->handleEvents(event);
            } else if (currentState == PLAY) {
                play->handleEvent(event);
            }
            render();
        }

        update();
        render();
        SDL_Delay(16);  // Giữ ~60 FPS
    }
}

void Game::openURL(const std::string& url) {
    #if defined(_WIN32)
        std::string command = "start " + url;
    #elif defined(__APPLE__)
        std::string command = "open " + url;
    #elif defined(__linux__)
        std::string command = "xdg-open " + url;
    #else
        #error "Unsupported OS"
    #endif

    system(command.c_str());
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            isRunning = false;
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
            int x = event.button.x;
            int y = event.button.y;

            if (currentState == MENU) {
                for (int i = 0; i < 4; i++) {
                    if (x >= menu->buttons[i].x && x <= menu->buttons[i].x + menu->buttons[i].w &&
                        y >= menu->buttons[i].y && y <= menu->buttons[i].y + menu->buttons[i].h) {

                        if (i == 1) {  // Nếu nhấn vào "Bản đồ"
                            SDL_Log("Switching to Level Select...");
                            levelSelect->loadLevels("assets/data/levels.json"); // Load dữ liệu trước khi chuyển
                            currentState = LEVEL_SELECT;
                        }
                    }
                }
            } else if (currentState == LEVEL_SELECT) {
                if (x >= levelSelect->backButton.x && x <= levelSelect->backButton.x + 200 &&
                    y >= levelSelect->backButton.y && y <= levelSelect->backButton.y + 80) {
                    SDL_Log("Returning to menu...");
                    currentState = MENU;
                }
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
        if (menu == nullptr) {
            menu = new Menu(renderer, &isRunning, this);
        }
        menu->render();
    } else if (currentState == LEVEL_SELECT) {
        levelSelect->render();
    } else if (currentState == PLAY) {
        if (play == nullptr) {
            play = new Play(renderer, &isRunning, this);
        }
        play->render();
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

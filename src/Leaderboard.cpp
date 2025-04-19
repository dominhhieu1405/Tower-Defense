#include "Leaderboard.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <fstream>
#include <algorithm>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

Leaderboard::Leaderboard(SDL_Renderer* renderer, bool* isRunning, Game* game) : renderer(renderer), game(game), isRunning(isRunning) {

    // Load hình nền
    backgroundTexture = IMG_LoadTexture(renderer, "assets/images/background.png");
    if (!backgroundTexture) {
        SDL_Log("Failed to load background: %s", SDL_GetError());
    }

    // Load board
    tableTexture = IMG_LoadTexture(renderer, "assets/images/wood.png");
    if (!tableTexture) {
        SDL_Log("Failed to load board: %s", SDL_GetError());
    }

    // Load button
    buttonTexture = IMG_LoadTexture(renderer, "assets/images/button.png");
    if (!buttonTexture) {
        SDL_Log("Failed to load button: %s", SDL_GetError());
    }


    clickSound = Mix_LoadWAV("assets/audios/click.wav");
    if (!clickSound) {
        SDL_Log("Failed to load click sound: %s", Mix_GetError());
    }

    font = TTF_OpenFont("assets/fonts/wood.ttf", 24);
    font2 = TTF_OpenFont("assets/fonts/wood.ttf", 50);
    if (!font) {
        SDL_Log("Không thể tải font! Lỗi: %s", TTF_GetError());
    }
    loadScores();
}

Leaderboard::~Leaderboard() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(buttonTexture);
    SDL_DestroyTexture(tableTexture);
    if (font) TTF_CloseFont(font);
    if (font2) TTF_CloseFont(font2);
    Mix_FreeChunk(clickSound);
    Mix_CloseAudio();
}

void Leaderboard::handleEvents(SDL_Event& event) {

    if (event.type == SDL_MOUSEMOTION || event.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = event.motion.x;
        int mouseY = event.motion.y;
        hoveredButton = -1;


        if (565 <= mouseX && mouseX <= 715 && 714 <= mouseY && mouseY <= 774) {
            hoveredButton = 1; // Nút quay lại

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                game->leaderboard = nullptr;
                game->currentState = MENU; // Quay lại menu
                // Xử lý click vào từng nút nếu cần
                if (clickSound) {
                    Mix_PlayChannel(-1, clickSound, 0);
                }
            }
        } else {
            hoveredButton = -1;
        }
    }
}

void Leaderboard::render() {

    int screenWidth, screenHeight;
    SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

    // Vẽ nền
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

//    // Hiển thị logo (căn giữa ngang, lệch lên trên)
//    const int logoWidth = 500;
//    const int logoHeight = 250;
//    SDL_Rect logoRect = {
//            (1280 - logoWidth) / 2,  // Giữa theo chiều ngang
//            35,  // Cách mép trên 35px
//            logoWidth,
//            logoHeight
//    };
//    SDL_RenderCopy(renderer, logoTexture, NULL, &logoRect);
    // Vẽ chữ Leaderboard
    SDL_Surface* textSurface_ = TTF_RenderUTF8_Blended(font2, "BẢNG XẾP HẠNG ONLINE", {0, 0, 0});
    SDL_Texture* textTexture_ = SDL_CreateTextureFromSurface(renderer, textSurface_);
    SDL_Rect textRect_ = {
            (screenWidth - textSurface_->w) / 2,  // Giữa theo chiều ngang
            80,  // Cách mép trên 50px
            textSurface_->w,
            textSurface_->h
    };
    SDL_RenderCopy(renderer, textTexture_, NULL, &textRect_);

    // Vẽ bảng
    const int tableWidth = 670;
    const int tableHeight = 536;
    SDL_Rect tableRect = {
            (screenWidth - tableWidth) / 2,  // Giữa theo chiều ngang
            (screenHeight - tableHeight) / 2,  // Giữa theo chiều dọc
            tableWidth,
            tableHeight
    };
    SDL_RenderCopy(renderer, tableTexture, NULL, &tableRect);

    int y = tableRect.y + 25;  // bắt đầu từ trên bảng xuống 30px
    const int lineHeight = 40;
    const int paddingLeft = tableRect.x + 40;       // rank và tên lệch trái trong bảng
    const int paddingRight = tableRect.x + tableRect.w - 40; // level lệch phải trong bảng

    for (const auto& line : ranks) {
        // Nếu dòng là gạch ngăn cách, in giữa
        if (line == "---------------------") {
            SDL_Surface* dashSurface = TTF_RenderUTF8_Blended(font, line.c_str(), {200, 200, 200});
            SDL_Texture* dashTexture = SDL_CreateTextureFromSurface(renderer, dashSurface);
            SDL_Rect dashRect = {
                    tableRect.x + (tableRect.w - dashSurface->w) / 2,
                    y,
                    dashSurface->w,
                    dashSurface->h
            };
            SDL_RenderCopy(renderer, dashTexture, NULL, &dashRect);
            SDL_FreeSurface(dashSurface);
            SDL_DestroyTexture(dashTexture);
            y += lineHeight;
            continue;
        }

        // Tách dòng "rank. name | Lv xx"
        size_t posLv = line.find("| Level");
        std::string leftText = line.substr(0, posLv);  // "rank. name"
        std::string rightText = line.substr(posLv + 2); // "Lv xx"

        // Render bên trái (rank + name)
        SDL_Surface* leftSurface = TTF_RenderUTF8_Blended(font, leftText.c_str(), {255, 255, 255});
        SDL_Texture* leftTexture = SDL_CreateTextureFromSurface(renderer, leftSurface);
        SDL_Rect leftRect = {paddingLeft, y, leftSurface->w, leftSurface->h};
        SDL_RenderCopy(renderer, leftTexture, NULL, &leftRect);
        SDL_FreeSurface(leftSurface);
        SDL_DestroyTexture(leftTexture);

        // Render bên phải (Lv xx)
        SDL_Surface* rightSurface = TTF_RenderUTF8_Blended(font, rightText.c_str(), {255, 255, 255});
        SDL_Texture* rightTexture = SDL_CreateTextureFromSurface(renderer, rightSurface);
        SDL_Rect rightRect = {
                paddingRight - rightSurface->w,
                y,
                rightSurface->w,
                rightSurface->h
        };
        SDL_RenderCopy(renderer, rightTexture, NULL, &rightRect);
        SDL_FreeSurface(rightSurface);
        SDL_DestroyTexture(rightTexture);

        y += lineHeight;
    }


    SDL_Rect backButton = { (1280 - 150) / 2, (screenHeight + tableHeight) / 2 + 20, 150, 60 };
    SDL_RenderCopy(renderer, buttonTexture, NULL, &backButton);
    SDL_Color textColor = { 255, 255, 255, 255 };
    if (hoveredButton == 1) {
        SDL_SetTextureColorMod(buttonTexture, 180, 180, 180);  // Giảm sáng xuống 180/255
    } else {
        SDL_SetTextureColorMod(buttonTexture, 255, 255, 255);  // Trả về màu gốc
    }
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, "QUAY LẠI", textColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = { (backButton.x + 150 / 2) - textSurface->w / 2, (backButton.y + 60 / 2 - 5) - textSurface->h / 2, textSurface->w, textSurface->h };

    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);

    SDL_RenderPresent(renderer);  // Cập nhật màn hình
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

void Leaderboard::loadScores() {
    ranks.clear();

    // Đọc uuid từ file
    std::ifstream inFile(game->dataPath);
    nlohmann::json data;
    inFile >> data;
    inFile.close();

    std::string uuid = data.contains("uuid") ? data["uuid"].get<std::string>() : "";

    std::string url = "https://towerdefense.bk25nkc.com/api/rank.php";
    if (!uuid.empty()) {
        url += "?uuid=" + uuid;
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5); // tránh đơ

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            try {
                nlohmann::json jsonResp = nlohmann::json::parse(response);
                //SDL_Log("player_rank: %s", jsonResp.dump().c_str());
                int rank = 1;
                for (const auto& player : jsonResp["top_players"]) {
                    std::string line = std::to_string(rank++) + ". " + player["name"].get<std::string>() +
                                       " | Level " + std::to_string(player["level"].get<int>());
                    ranks.push_back(line);
                    //SDL_Log("Line: %s", line.c_str());
                }

                if (jsonResp.contains("player_rank") && !jsonResp["player_rank"].is_null()) {
                    ranks.push_back("---------------------");
                    SDL_Log("player_rank: %s", jsonResp["player_rank"].dump().c_str());
                    nlohmann::json you = jsonResp["player_rank"];
                    std::string line = std::to_string(you["rank"].get<int>()) + ". " + you["name"].get<std::string>() +
                                       " | Level " + std::to_string(you["level"].get<int>());
                    ranks.push_back(line);
                    //SDL_Log("Line: %s", line.c_str());
                }
            } catch (std::exception& e) {
                SDL_Log("Lỗi phân tích JSON: %s", e.what());
            }
        } else {
            SDL_Log("Lỗi curl: %s", curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
    } else {
        SDL_Log("Không khởi tạo được CURL.");
    }
}
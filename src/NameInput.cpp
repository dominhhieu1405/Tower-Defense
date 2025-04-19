#include "NameInput.h"
#include "Game.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <fstream>

static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    if (!ptr || !userdata) return 0;
    try {
        std::string* response = static_cast<std::string*>(userdata);
        response->append(ptr, size * nmemb);
    } catch (...) {
        SDL_Log("Exception in WriteCallback");
    }
    return size * nmemb;
}


NameInput::NameInput(SDL_Renderer* r, bool* run, Game* g, const std::string& path)
        : renderer(r), isRunning(run), game(g), dataPath(path), textTexture(nullptr)
{

    // Load hình nền
    backgroundTexture = IMG_LoadTexture(renderer, "assets/images/background.png");
    if (!backgroundTexture) {
        SDL_Log("Failed to load background: %s", SDL_GetError());
    }

    // Load textbox
    inputTexture = IMG_LoadTexture(renderer, "assets/images/textbox.png");
    if (!inputTexture) {
        SDL_Log("Failed to load textbox: %s", SDL_GetError());
    }

    // Load logo
    logoTexture = IMG_LoadTexture(renderer, "assets/images/logo.png");
    if (!logoTexture) {
        SDL_Log("Failed to load logo: %s", SDL_GetError());
    }


    clickSound = Mix_LoadWAV("assets/audios/keydown.wav");
    if (!clickSound) {
        SDL_Log("Failed to load click sound: %s", Mix_GetError());
    }

    font = TTF_OpenFont("assets/fonts/text.ttf", 32);
    font2 = TTF_OpenFont("assets/fonts/wood.ttf", 50);
    inputText = "";
    inputBox = {1080/2, 440, 200, 50};
    //SDL_StartTextInput();
    updateTexture();
}

NameInput::~NameInput() {
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(logoTexture);
    if (textTexture) SDL_DestroyTexture(textTexture);
    if (font) TTF_CloseFont(font);
    if (font2) TTF_CloseFont(font2);
    Mix_FreeChunk(clickSound);
    Mix_CloseAudio();
}

void NameInput::handleEvent(SDL_Event& e) {
    if (e.type == SDL_TEXTINPUT) {
        SDL_Log("Text input: %s", e.text.text);
        // Kiểm tra nếu có văn bản nhập vào
        if (e.text.text[0] == '\0') return;  // Không làm gì nếu không có văn bản

        // Thêm văn bản vào inputText
        inputText += e.text.text;

        if (clickSound) {
            Mix_PlayChannel(-1, clickSound, 0);
        }
    }
    if (e.type == SDL_KEYDOWN) {
        //std::cout << "Key down: " << (e.key.keysym.sym) << std::endl;

        // Kiểm tra phím Backspace để xóa ký tự cuối
        if (e.key.keysym.sym == SDLK_BACKSPACE && !inputText.empty()) {
            inputText.pop_back();  // Xóa ký tự cuối cùng

            if (clickSound) {
                Mix_PlayChannel(-1, clickSound, 0);
            }
        }
            // Kiểm tra phím Enter để lưu tên và chuyển trạng thái game
        else if (e.key.keysym.sym == SDLK_RETURN && !inputText.empty()) {
            saveName();
            game->currentState = MENU;
        }
            // Kiểm tra các phím hợp lệ (a-z, A-Z, 0-9, _ , - và dấu cách)
        else if ((e.key.keysym.sym >= SDLK_a && e.key.keysym.sym <= SDLK_z) ||
                 (e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9) ||
                 e.key.keysym.sym == SDLK_SPACE ||  // Dấu cách
                 e.key.keysym.sym == SDLK_MINUS || // Dấu "-"
                 e.key.keysym.sym == SDLK_UNDERSCORE) { // Dấu "_"
            // Thêm ký tự vào inputText
            if (SDLK_SPACE == e.key.keysym.sym) {
                inputText += " ";  // Thêm dấu cách
            } else {
                // Chuyển đổi ký tự thành chữ thường nếu cần
                if (e.key.keysym.mod & KMOD_SHIFT) { // Chữ hoa
                    inputText += SDL_GetKeyName(e.key.keysym.sym);  // Thêm ký tự vào chuỗi
                } else { // Chữ thường
                    inputText += tolower(SDL_GetKeyName(e.key.keysym.sym)[0]);  // Thêm ký tự vào chuỗi
                }
            }

            if (clickSound) {
                Mix_PlayChannel(-1, clickSound, 0);
            }
            //inputText += SDL_GetKeyName(e.key.keysym.sym);  // Thêm ký tự vào chuỗi
            //SDL_Log("%s", inputText.c_str());
        }

        updateTexture();  // Cập nhật texture sau khi thay đổi inputText
    }
}



void NameInput::updateTexture() {
    if (textTexture) SDL_DestroyTexture(textTexture);

    if (inputText.empty()) {
        std::cout << "InputText is empty!" << std::endl;
        return;
    }

    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font2, inputText.c_str(), color);

    if (!surface) {
        std::cerr << "Error creating surface: " << TTF_GetError() << std::endl;
        return;
    }

    const int maxWidth = 700;
    SDL_Surface* croppedSurface = surface;

    if (surface->w > maxWidth) {
        // Tạo surface mới để chứa phần crop
        croppedSurface = SDL_CreateRGBSurfaceWithFormat(0, maxWidth, surface->h, 32, surface->format->format);
        if (croppedSurface) {
            SDL_Rect srcRect = {surface->w - maxWidth, 0, maxWidth, surface->h};  // Crop từ phải
            SDL_BlitSurface(surface, &srcRect, croppedSurface, nullptr);
        }
    }

    textTexture = SDL_CreateTextureFromSurface(renderer, croppedSurface);

    inputBox.w = std::min(surface->w, maxWidth) + 10;
    inputBox.h = surface->h + 10;
    inputBox.x = (1280 - inputBox.w) / 2;

    SDL_FreeSurface(surface);
    if (croppedSurface != surface) SDL_FreeSurface(croppedSurface);
}




void NameInput::render() {

    int screenWidth, screenHeight;
    SDL_GetRendererOutputSize(renderer, &screenWidth, &screenHeight);

    // Vẽ nền
    SDL_RenderCopy(renderer, backgroundTexture, NULL, NULL);

    // Hiển thị logo (căn giữa ngang, lệch lên trên)
    const int logoWidth = 500;
    const int logoHeight = 250;
    SDL_Rect logoRect = {
            (1280 - logoWidth) / 2,  // Giữa theo chiều ngang
            35,  // Cách mép trên 35px
            logoWidth,
            logoHeight
    };
    SDL_RenderCopy(renderer, logoTexture, NULL, &logoRect);

    // Hiển thị ô input
    SDL_Rect inputRect = {(1280 - 800)/2, 400, 800, 200};
    SDL_RenderCopy(renderer, inputTexture, NULL, &inputRect);

    SDL_Color color = {0, 0, 0, 255};
    SDL_Surface* label = TTF_RenderUTF8_Blended(font, "Tên bạn là gì?", color);
    SDL_Texture* labelTexture = SDL_CreateTextureFromSurface(renderer, label);
    SDL_Rect labelRect = {(1280 - label->w)/2, 400, label->w, label->h};
    SDL_RenderCopy(renderer, labelTexture, nullptr, &labelRect);
    SDL_FreeSurface(label);
    SDL_DestroyTexture(labelTexture);

    //SDL_RenderDrawRect(renderer, &inputBox);  // Vẽ viền ô nhập liệu
    if (textTexture) {
        SDL_RenderCopy(renderer, textTexture, nullptr, &inputBox);  // Vẽ nội dung văn bản vào ô nhập liệu
    }

    SDL_RenderPresent(renderer);  // Cập nhật màn hình
}


void NameInput::saveName() {
    // Đọc file JSON
    std::ifstream inFile(dataPath);
    nlohmann::json data;
    inFile >> data;
    inFile.close();

    std::string uuid = data.contains("uuid") ? data["uuid"].get<std::string>() : "";
    std::string url;
    std::string postData;

    if (!uuid.empty()) {
        url = "https://towerdefense.bk25nkc.com/api/change_name.php";
        postData = "name=" + inputText + "&uuid=" + uuid;
        SDL_Log("Sending change_name request: %s", postData.c_str());
    } else {
        url = "https://towerdefense.bk25nkc.com/api/join.php";
        postData = "name=" + inputText;
        SDL_Log("Sending join request: %s", postData.c_str());
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        CURLcode res;
        std::string response;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());

        // Callback nhận phản hồi
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            SDL_Log("curl_easy_perform() failed: %s", curl_easy_strerror(res));
        } else {
            SDL_Log("Response from server: %s", response.c_str());
            if (uuid.empty()) {
                try {
                    nlohmann::json respJson = nlohmann::json::parse(response);
                    if (respJson.contains("uuid")) {
                        data["uuid"] = respJson["uuid"];
                        SDL_Log("Received new UUID: %s", respJson["uuid"].get<std::string>().c_str());
                    } else {
                        SDL_Log("Response JSON doesn't contain UUID.");
                    }
                } catch (std::exception& e) {
                    SDL_Log("JSON parse error: %s", e.what());
                }
            } else {
                SDL_Log("Name update sent successfully.");
            }
        }

        curl_easy_cleanup(curl);
    } else {
        SDL_Log("Failed to initialize CURL.");
    }

    // Luôn lưu tên vào JSON dù CURL có lỗi hay không
    data["name"] = inputText;
    std::ofstream outFile(dataPath);
    outFile << data.dump(4);
    outFile.close();

    SDL_Log("Saved name: %s", inputText.c_str());
}

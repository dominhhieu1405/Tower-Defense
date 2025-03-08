#include "play.h"
#include "Tower.h"
#include <fstream>
#include <nlohmann/json.hpp>

Play::Play(SDL_Renderer* renderer, bool* isRunning, Game* game)
        : renderer(renderer), isRunning(isRunning), game(game), tilesetTexture(nullptr) {
    loadMap(game->selectedLevel);
    startTick = SDL_GetTicks();
    lastFrameTime = SDL_GetTicks();
    currentFrame = 0;
    money = 200;
    lives = 5;

    gameFont = TTF_OpenFont("assets/fonts/wood.ttf", 24); // Kích thước 24px
    if (!gameFont) {
        printf("Failed to load font: %s\n", TTF_GetError());
    }
}

Play::~Play() {
    if (tilesetTexture) {
        SDL_DestroyTexture(tilesetTexture);
    }
}

SDL_Texture* Play::loadTexture(const std::string& path) {
    SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
    if (!texture) {
        SDL_Log("Failed to load texture: %s, Error: %s", path.c_str(), SDL_GetError());
    }
    return texture;
}

void Play::loadMap(int levelID) {
    std::string mapPath = "assets/maps/" + std::to_string(levelID) + ".json";
    parseMapJson(mapPath);
    tilesetTexture = loadTexture("assets/images/Tileset/tileset.png");
}

void Play::parseMapJson(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file) {
        SDL_Log("Failed to open map file: %s", filePath.c_str());
        *isRunning = false; // Dừng game nếu không thể tải bản đồ
        return;
    }

    nlohmann::json jsonData;
    file >> jsonData;
    file.close();

    if (!jsonData.contains("layers") || !jsonData["layers"].is_array()) {
        SDL_Log("Invalid map JSON format");
        *isRunning = false;
        return;
    }

    for (const auto& layer : jsonData["layers"]) {
        if (layer.contains("data") && layer["data"].is_array()) {
            mapData.clear();
            std::vector<int> row;
            int colCount = 0;
            for (const auto& tile : layer["data"]) {
                if (tile.is_number_integer()) {
                    row.push_back(tile.get<int>());
                } else {
                    row.push_back(0); // Tránh lỗi nếu dữ liệu null
                }
                colCount++;
                if (colCount == MAP_WIDTH) {
                    mapData.push_back(row);
                    row.clear();
                    colCount = 0;
                }
            }
            break; // Chỉ lấy layer đầu tiên
        }
    }
}

void Play::handleEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) {
                int clickX = event.button.x;
                int clickY = event.button.y;

                int bottomPanelY = MAP_HEIGHT * TILE_SIZE; // Vị trí y bắt đầu vùng hiển thị tower
                int towerY = bottomPanelY + (149 - 128) / 2;
                int towerX = 256;
                const int towerSpacing = 128;

                // Kiểm tra nếu nhấn vào một tower
                for (auto& tower : game->towerManager.towers) {
                    int towerStartX = towerX + (128 - 64) / 2;
                    if (clickX >= towerStartX && clickX <= towerStartX + 64 &&
                        clickY >= towerY && clickY <= towerY + 128) {
                        isDragging = true;
                        draggedTower = &tower;
                        mouseX = clickX;
                        mouseY = clickY;
                        //SDL_Log("Dragging Tower!");
                        break;
                    }
                    towerX += towerSpacing;
                }
            }
            break;

        case SDL_MOUSEMOTION:
            if (isDragging && draggedTower) {
                mouseX = event.motion.x;
                mouseY = event.motion.y; // Căn giữa con trỏ chuột

                if (mouseX > 0 && mouseY > 0 && mouseX < MAP_WIDTH * TILE_SIZE && mouseY < MAP_HEIGHT * TILE_SIZE - 49 ) {
                    // Xác định tile gần nhất
                    int tileX = mouseX / TILE_SIZE;
                    int tileY = (mouseY + 49) / TILE_SIZE; // Lệch xuống dưới 49px, căn theo chân towere

                    // Kiểm tra tile có hợp lệ không
                    int tileIndex = mapData[tileY][tileX]; // Lấy giá trị ô
                    highlightTileX = tileX * TILE_SIZE;
                    highlightTileY = tileY * TILE_SIZE;
                    highlightValid = std::find(std::begin(allowTiles), std::end(allowTiles), tileIndex) != std::end(allowTiles);
                } else {
                    highlightTileX = -1;
                    highlightTileY = -1;
                    highlightValid = false;
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                isDragging = false;
                draggedTower = nullptr;
            }
            break;
    }
}


void Play::render() {
    if (!tilesetTexture || mapData.empty()) return;

    Uint32 now = SDL_GetTicks();
    if (now - lastFrameTime >= 200) {  // Cập nhật frame sau mỗi 200ms
        lastFrameTime = now;
        currentFrame++;
        // Nếu max int thì reset về 0
        if (currentFrame >= 1000000) {
            currentFrame = 0;
        }
    }

    SDL_Rect srcRect = {0, 0, TILE_SIZE, TILE_SIZE};
    SDL_Rect destRect = {0, 0, TILE_SIZE, TILE_SIZE};

    // Vẽ bản đồ
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            int tileIndex = mapData[y][x] - 1;
            if (tileIndex < 0) continue;

            srcRect.x = (tileIndex % TILESET_COLS) * TILE_SIZE;
            srcRect.y = (tileIndex / TILESET_COLS) * TILE_SIZE;
            destRect.x = x * TILE_SIZE;
            destRect.y = y * TILE_SIZE;

            SDL_RenderCopy(renderer, tilesetTexture, &srcRect, &destRect);
        }
    }

    // Vẽ danh sách các Tower dưới cùng
    int bottomPanelY = MAP_HEIGHT * TILE_SIZE;
    int towerY = bottomPanelY + (149 - 128) / 2;
    int towerX = 256; // Bắt đầu từ vị trí 256px
    const int towerSpacing = 128;

    // Vẽ nền panel
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect panelRect = {0, bottomPanelY, 1280, 149};
    SDL_RenderFillRect(renderer, &panelRect);


    for (const auto& tower : game->towerManager.towers) {
        if (tower.texture) {
            SDL_Rect towerSrc = {0, 0, 64, 128};
            SDL_Rect towerDest = {
                    towerX + (towerSpacing - 64) / 2,
                    towerY,
                    64, 128
            };
            SDL_RenderCopy(renderer, tower.texture, &towerSrc, &towerDest);


            // Lấy weapon của Level 1 (hoặc level hiện tại)
            const TowerLevel& level = tower.levels[0];
            if (level.weapon.path != "") {
                int weaponWidth = level.weapon.frameWidth / level.weapon.frameCount;
                int weaponHeight = level.weapon.frameHeight;

                SDL_Rect weaponSrc = {(currentFrame % level.weapon.frameCount) * weaponWidth, 0, weaponWidth, weaponHeight};
                SDL_Rect weaponDest = {
                        towerDest.x + (towerDest.w - weaponWidth) / 2,
                        towerDest.y,  // Chống lên tower
                        weaponWidth, weaponHeight
                };

                SDL_RenderCopy(renderer, level.weapon.texture, &weaponSrc, &weaponDest);
            }

            towerX += towerSpacing;
        }
    }

    // Vẽ bản sao Tower nếu đang kéo
    if (isDragging && draggedTower) {

        // Vẽ lớp phủ màu đỏ trong suốt 50% lên tile gần nhất

        if (highlightTileX >= 0 && highlightTileY >= 0) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            if (highlightValid) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 128); // Màu xanh, alpha 128 = 50% trong suốt
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 128); // Màu đỏ, alpha 128 = 50% trong suốt
            }
            SDL_Rect highlightRect = {highlightTileX, highlightTileY, TILE_SIZE, TILE_SIZE};
            SDL_RenderFillRect(renderer, &highlightRect);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        }


        SDL_Rect towerSrc = {0, 0, 64, 128};
        SDL_Rect draggedRect = {mouseX - 32, mouseY - 64, 64, 128}; // Căn giữa con trỏ chuột
        SDL_RenderCopy(renderer,  draggedTower->texture, &towerSrc, &draggedRect);

        // Lấy weapon của Level 1 (hoặc level hiện tại)
        const TowerLevel& level = draggedTower->levels[0];
        if (level.weapon.path != "") {
            int weaponWidth = level.weapon.frameWidth / level.weapon.frameCount;
            int weaponHeight = level.weapon.frameHeight;

            SDL_Rect weaponSrc = {0, 0, weaponWidth, weaponHeight};
            SDL_Rect weaponDest = {
                    draggedRect.x + (draggedRect.w - weaponWidth) / 2,
                    draggedRect.y,  // Chống lên tower
                    weaponWidth, weaponHeight
            };

            SDL_RenderCopy(renderer, level.weapon.texture, &weaponSrc, &weaponDest);
        }
    }



    int timePlayed =  int ((SDL_GetTicks() - startTick) / 1000);

    int minutes = timePlayed / 60;
    int seconds = timePlayed % 60;
    char timeText[20];
    snprintf(timeText, sizeof(timeText), u8"Thời gian: %02d:%02d", minutes, seconds);
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(gameFont, timeText, {255, 255, 255, 255});



    SDL_Texture* timeTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    // Hiển thị ở góc trên bên trái
    SDL_Rect timeRect = {10, bottomPanelY, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, timeTexture, NULL, &timeRect);


    char moneyText[20];
    snprintf(moneyText, sizeof(moneyText), u8"Tiền: %03d", money);
    SDL_Surface* textSurface2 = TTF_RenderUTF8_Blended(gameFont, moneyText, {255, 255, 255, 255});
    SDL_Texture* moneyTexture = SDL_CreateTextureFromSurface(renderer, textSurface2);
    SDL_FreeSurface(textSurface2);

    char livesText[20];
    snprintf(livesText, sizeof(livesText), u8"Mạng: %02d", lives);
    SDL_Surface* textSurface3 = TTF_RenderUTF8_Blended(gameFont, livesText, {255, 255, 255, 255});
    SDL_Texture* livesTexture = SDL_CreateTextureFromSurface(renderer, textSurface3);
    SDL_FreeSurface(textSurface3);

    SDL_Rect moneyRect = {10, bottomPanelY + 45, textSurface2->w, textSurface2->h};
    SDL_RenderCopy(renderer, moneyTexture, NULL, &moneyRect);

    SDL_Rect livesRect = {10, bottomPanelY + 90, textSurface3->w, textSurface3->h};
    SDL_RenderCopy(renderer, livesTexture, NULL, &livesRect);
}


void Play::update() {
    // Cập nhật game logic nếu cần
}

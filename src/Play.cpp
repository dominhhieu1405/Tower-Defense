#include "play.h"
#include "Tower.h"
#include <fstream>
#include <nlohmann/json.hpp>

Play::Play(SDL_Renderer* renderer, bool* isRunning, Game* game)
        : renderer(renderer), isRunning(isRunning), game(game), tilesetTexture(nullptr) {
    loadMap(game->selectedLevel);

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
                int towerX = 10;
                const int towerSpacing = 149;

                // Kiểm tra nếu nhấn vào một tower
                for (auto& tower : game->towerManager.towers) {
                    int towerStartX = towerX + (149 - 64) / 2;
                    if (clickX >= towerStartX && clickX <= towerStartX + 64 &&
                        clickY >= towerY && clickY <= towerY + 128) {
                        isDragging = true;
                        draggedTower = &tower;
                        mouseX = clickX;
                        mouseY = clickY;
                        SDL_Log("Dragging Tower!");
                        break;
                    }
                    towerX += towerSpacing;
                }
            }
            break;

        case SDL_MOUSEMOTION:
            if (isDragging && draggedTower) {
                mouseX = event.motion.x;
                mouseY = event.motion.y;

                if (mouseX > 0 && mouseY > 0 && mouseX < MAP_WIDTH * TILE_SIZE && mouseY < MAP_HEIGHT * TILE_SIZE) {
                    // Xác định tile gần nhất
                    int tileX = mouseX / TILE_SIZE;
                    int tileY = mouseY / TILE_SIZE;

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
    int towerX = 10;
    const int towerSpacing = 149;

    // Vẽ nền panel
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Rect panelRect = {0, bottomPanelY, 1280, 149};
    SDL_RenderFillRect(renderer, &panelRect);


    for (const auto& tower : game->towerManager.towers) {
        if (tower.texture) {
            SDL_Rect towerSrc = {0, 0, 64, 128};
            SDL_Rect towerDest = {
                    towerX + (149 - 64) / 2,
                    towerY,
                    64, 128
            };
            SDL_RenderCopy(renderer, tower.texture, &towerSrc, &towerDest);
            towerX += towerSpacing;
        }
    }

    // Vẽ bản sao Tower nếu đang kéo
    if (isDragging && draggedTower) {
        SDL_Rect towerSrc = {0, 0, 64, 128};
        SDL_Rect draggedRect = {mouseX - 32, mouseY - 64, 64, 128}; // Căn giữa con trỏ chuột
        SDL_RenderCopy(renderer,  draggedTower->texture, &towerSrc, &draggedRect);
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
    }
}


void Play::update() {
    // Cập nhật game logic nếu cần
}

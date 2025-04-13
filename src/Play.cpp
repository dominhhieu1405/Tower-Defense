#include "Play.h"
#include "Tower.h"
#include "Enemy.h"
#include "playTower.h"
#include <fstream>
#include <nlohmann/json.hpp>

Play::Play(SDL_Renderer* renderer, bool* isRunning, Game* game)
        : renderer(renderer), isRunning(isRunning), game(game), tilesetTexture(nullptr) {
    loadMap(game->selectedLevel);
    startTick = SDL_GetTicks();
    lastFrameTime = SDL_GetTicks();
    currentFrame = 0;
    money = 2000000;
    lives = 5;

    gameFont = TTF_OpenFont("assets/fonts/wood.ttf", 24); // Kích thước 24px
    if (!gameFont) {
        printf("Failed to load font: %s\n", TTF_GetError());
    }

    placeSound = Mix_LoadWAV("assets/audios/thud.wav");
    if (!placeSound) {
        SDL_Log("Failed to load click sound: %s", Mix_GetError());
    }
}

Play::~Play() {
    if (tilesetTexture) {
        SDL_DestroyTexture(tilesetTexture);
    }

    Mix_FreeChunk(placeSound);
    Mix_CloseAudio();
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

    std::string dataPath = "assets/data/levels/" + std::to_string(levelID) + ".json";
    std::ifstream file(dataPath);
    if (!file) {
        SDL_Log("Failed to open level data file: %s", dataPath.c_str());
        *isRunning = false; // Dừng game nếu không tải được dữ liệu level
        return;
    }
    file >> levelData;
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

                    // Kiểm tra xem có đặt tower lên tower cũ không
                    for (int i = 0; i < (int) towers.size(); i++) {
                        playTower& tower = towers[i];
                        if (tower.x == tileX && tower.y == tileY) {
                            highlightValid = false;
                            break;
                        }
                    }

                } else {
                    highlightTileX = -1;
                    highlightTileY = -1;
                    highlightValid = false;
                }
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) {
                // console log
                if (isDragging) {
                    // Kiểm tra chỗ đặt có hợp lệ không
                    if (highlightValid) {
                        // Kiểm tra xem có đủ tiền không
                        if (money < draggedTower->cost[0]) {
                            SDL_Log("Không đủ tiền mua tower!");
                            break;
                        }

                        // Kiểm tra xem có đặt tower lên tower cũ không
                        // không dùng auto
                        for (int i = 0; i < (int) towers.size(); i++) {
                            playTower& tower = towers[i];
                            if (tower.x == highlightTileX / TILE_SIZE && tower.y == highlightTileY / TILE_SIZE) {

//                                // Nếu đặt tower lên tower cũ thì nâng cấp tower
//                                if (tower.tower == draggedTower) {
//                                    if (tower.level < 2) {
//                                        // Kieemr tra xem có đủ tiền nâng cấp không
//                                        if (money < draggedTower->cost[tower.level + 1]) {
//                                            SDL_Log("Không đủ tiền nâng cấp tower!");
//                                            break;
//                                        }
//                                        tower.level = tower.level + 1;
//                                        SDL_Log("Nâng cấp tower lên level %d", tower.level + 1);
//                                        // Trừ tiền
//                                        money -= draggedTower->cost[tower.level];
//                                    } else {
//                                        SDL_Log("Đã đạt cấp độ tối đa!");
//                                    }
//                                } else {
//                                    SDL_Log("Không thể đặt tower lên tower khác!");
//                                }
                                SDL_Log("Không thể đặt tower lên tower khác!");
                                return;
                            }
                        }

                        // Kiểm tra xem có đặt tower lên đường không
                        int tileIndex = mapData[highlightTileY / TILE_SIZE][highlightTileX / TILE_SIZE];
                        if (std::find(std::begin(allowTiles), std::end(allowTiles), tileIndex) == std::end(allowTiles)) {
                            SDL_Log("Không thể đặt tower lên đường!");
                            return;
                        }

                        // Đặt tower
                        SDL_Log("Highlight tile: %d, %d", highlightTileX/TILE_SIZE, highlightTileY/TILE_SIZE);
                        playTower newTower;
                        newTower.tower = draggedTower;
                        newTower.x = highlightTileX / TILE_SIZE;
                        newTower.y = highlightTileY / TILE_SIZE;
                        newTower.level = 0;
                        towers.push_back(newTower);

                        // Trừ tiền
                        money -= draggedTower->cost[0];

                        if (placeSound) {
                            Mix_PlayChannel(-1, placeSound, 0);
                        }

                    } else {
                        SDL_Log("Không thể đặt tower ở đây!");
                    }
                }

                isDragging = false;
                draggedTower = nullptr;
            }
            break;
    }
}

void Play::spawnEnemy() {
    int timePlayed =  int ((SDL_GetTicks() - startTick) / 1000);

    if (spawned.size() < levelData["total"]) {
        for (auto& stage: levelData["stages"]) {
            int stageTime = stage["time"];
            if (stageTime <= timePlayed) {
                for (auto& enemyData: stage["enemies"]) {
                    int enemyID = enemyData["id"];
                    int enemyType = enemyData["enemy"];
                    int spawnTime = enemyData["time"];
                    // Check nếu enemyID đã spawn thì skip
                    if (spawned.find(enemyID) == spawned.end()) {
                        if (stageTime + spawnTime <= timePlayed) {
                            // Spawn enemy
                            playEnemy newEnemy;
                            newEnemy.enemy = &(game->enemyManager.enemies.at(enemyType));
                            newEnemy.health = newEnemy.enemy->levels[0].health;
                            newEnemy.maxHealth = newEnemy.enemy->levels[0].health;
                            newEnemy.speed = newEnemy.enemy->levels[0].speed;
                            SDL_Log("Speed: %f", newEnemy.speed);
                            newEnemy.reward = newEnemy.enemy->levels[0].reward;
                            //newEnemy.tileX = enemyData["x"];
                            //newEnemy.tileY = enemyData["y"];
                            newEnemy.path = enemyData["path"];
                            newEnemy.rotation = enemyData["rotation"];

                            newEnemy.spawnTime = SDL_GetTicks();
                            newEnemy.lastMove = SDL_GetTicks();
                            newEnemy.tileX = levelData["paths"][newEnemy.path][0]["x"];
                            newEnemy.tileY = levelData["paths"][newEnemy.path][0]["y"];


                            if (newEnemy.rotation == 0) { // đi từ cạnh trên xuống
                                newEnemy.x = TILE_SIZE * newEnemy.tileX;
                                newEnemy.y = TILE_SIZE * (newEnemy.tileY - 1);
                            } else if (newEnemy.rotation == 90) { // đi từ cạnh phải qua trái
                                newEnemy.x = TILE_SIZE * (newEnemy.tileX + 1);
                                newEnemy.y = TILE_SIZE * newEnemy.tileY;
                            } else if (newEnemy.rotation == 180) { // đi từ cạnh dưới lên
                                newEnemy.x = TILE_SIZE * newEnemy.tileX;
                                newEnemy.y = TILE_SIZE * (newEnemy.tileY + 1);
                            } else if (newEnemy.rotation == 270) { // đi từ cạnh trái qua phải
                                newEnemy.x = TILE_SIZE * (newEnemy.tileX - 1);
                                newEnemy.y = TILE_SIZE * newEnemy.tileY;
                            }

                            newEnemy.level = enemyData["level"];
                            enemies.push_back(newEnemy);
                            spawned.insert(enemyID);

                            SDL_Log("Spawned enemy %d at %d", enemyID, timePlayed);
                        }
                    }
                }
            }
        }
    }
}
void Play::moveEnemies() {
    // spped = 1 là đi 64px / giây
    // cứ 1/8s thì di chuyển 8px

    // tốc độ quay: 90 độ / 0.3s
    // cứ 30ms quay được 9 độ

    // các hướng:
    // 0 độ: ĐI xuống dưới
    // 90 độ: Phải qua tráu
    // 180 độ: Lên trên
    // 270 độ: Trái qua phải

    for (int i = 0; i < (int) enemies.size(); i++) {
        playEnemy& enemy = enemies[i];
        if (enemy.status == 1) {
            Uint32 now = SDL_GetTicks();

            // kiểm tra hướng đi
            if (enemy.rotation >= 360) {
                enemy.rotation = enemy.rotation % 360;
            }

            // Kiểm tra xem ô hiện tại có phải ô cuối cùng không
            int totalPath = levelData["paths"][enemy.path].size();

            if(enemy.pathIndex == totalPath - 1) {
                // kiểm tra ID của tile ô cuối
                int tileIndex = mapData[enemy.tileY][enemy.tileX];

                if (tileIndex == 87) {
                    if (enemy.x <= (enemy.tileX) * TILE_SIZE) {
                        enemy.status = -1;
                        lives--;
                    }
                } else if (tileIndex == 184) {
                    if (enemy.x >= (enemy.tileX + 1) * TILE_SIZE) {
                        enemy.status = -1;
                        lives--;
                    }
                } else if (tileIndex == 70) {
                    if (enemy.y <= (enemy.tileY) * TILE_SIZE) {
                        enemy.status = -1;
                        lives--;
                    }
                } else { // tileIndex == 89
                    if (enemy.y >= (enemy.tileY) * TILE_SIZE) {
                        enemy.status = -1;
                        lives--;
                    }
                }
            }
            if (enemy.pathIndex < totalPath - 1) {
                int nextTileX = levelData["paths"][enemy.path][enemy.pathIndex + 1]["x"];
                int nextTileY = levelData["paths"][enemy.path][enemy.pathIndex + 1]["y"];

                // Nếu ô tiếp theo ở trên
                if (enemy.rotation % 90 == 0) {
                    enemy.tmpRotation = enemy.rotation;
                }
                int nextRotation = 0;
                if (nextTileX < enemy.tileX) {
                    nextRotation = 90;
                } else if (nextTileX > enemy.tileX) {
                    nextRotation = 270;
                } else if (nextTileY > enemy.tileY) {
                    nextRotation = 0;
                } else {
                    nextRotation = 180;
                }

                if ((enemy.tmpRotation == 90 && enemy.x >= (nextTileX-1) * TILE_SIZE) ||
                    (enemy.tmpRotation == 270 && enemy.x <= (nextTileX+1) * TILE_SIZE) ||
                    (enemy.tmpRotation == 0 && enemy.y <= nextTileY * TILE_SIZE) ||
                    (enemy.tmpRotation == 180 && enemy.y >= nextTileY * TILE_SIZE)) {

                    if (now - enemy.lastMove >= 30 && enemy.rotation != nextRotation) {
                        SDL_Log("Quay hướng mới: %d => %d", enemy.tmpRotation, nextRotation);
                        if ((enemy.tmpRotation == 90 && nextRotation == 180) || (enemy.tmpRotation == 180 && nextRotation == 270) ||
                            (enemy.tmpRotation == 270 && nextRotation == 0) || (enemy.tmpRotation == 0 && nextRotation == 90)) {
                            enemy.rotation += 9;
                        } else {
                            enemy.rotation -= 9;
                        }

                        enemy.lastMove = now;

                        // Kiểm tra xem đã quay đúng hướng chưa
                        if (enemy.rotation == nextRotation) {
                            enemy.tileX = nextTileX;
                            enemy.tileY = nextTileY;
                            enemy.pathIndex++;
                            enemy.tmpRotation = enemy.rotation;
                        }
                    }
                }

                if (enemy.rotation < 0) {
                    enemy.rotation += 360;
                }

                if (now - enemy.lastMove >= 62 && enemy.rotation % 90 == 0) {
                    enemy.lastMove = now;

                    // tốc độ mỗi 125ms
                    double speed = enemy.speed * 4.0;
                    int moveSpeed = (int) speed;


                    if (enemy.rotation == 0) {
                        enemy.y += moveSpeed;
                    } else if (enemy.rotation == 90) {
                        enemy.x -= moveSpeed;
                    } else if (enemy.rotation == 180) {
                        enemy.y -= moveSpeed;
                    } else if (enemy.rotation == 270) {
                        enemy.x += moveSpeed;
                    }

                    // Cập nhật pathIndex nếu đã đến gần trung tâm ô tiếp theo
                    if (abs(enemy.x - nextTileX * TILE_SIZE) < moveSpeed && abs(enemy.y - nextTileY * TILE_SIZE) < moveSpeed) {
                        enemy.tileX = nextTileX;
                        enemy.tileY = nextTileY;
                        enemy.pathIndex++;
                    }
                }

            }
        }
    }
}


void Play::renderEnemies() {
    for (int i = 0; i < (int) enemies.size(); i++) {
        playEnemy& enemy = enemies[i];
        if (enemy.status == 1) {
            int enemyCurrentFrame = (SDL_GetTicks() - enemy.spawnTime) / 125;

            SDL_Rect enemySrc = {(enemyCurrentFrame % enemy.enemy->levels[enemy.level].frame) * enemy.enemy->frameWidth, 3 * enemy.level * enemy.enemy->frameHeight, enemy.enemy->frameWidth, enemy.enemy->frameHeight};
            SDL_Rect enemyDest = {enemy.x - 64, enemy.y, TILE_SIZE, TILE_SIZE};
            // Quay 90 độ
            SDL_RenderCopyEx(renderer, enemy.enemy->texture, &enemySrc, &enemyDest, enemy.rotation, NULL, SDL_FLIP_NONE);
        }
    }
}

void Play::movePlayTowers() {

}

void Play::renderPlayTowers() {
    for (const auto& tower : towers) {
        SDL_Rect towerSrc = {0 * tower.level, 0, 64, 128}; // Lấy frame theo level
        // Vì tower height = 128 nên đặt lên trên 1 tile
        SDL_Rect towerDest = {
                tower.x * TILE_SIZE,
                tower.y * TILE_SIZE - 64, // Chống lên trên 1 tile
                64, 128
        };
        SDL_RenderCopy(renderer, tower.tower->texture, &towerSrc, &towerDest);

        // Lấy weapon của Level 1 (hoặc level hiện tại)
        const TowerLevel& level = tower.tower->levels[tower.level];
        if (level.weapon.path != "") {
            int weaponWidth = level.weapon.frameWidth / level.weapon.frameCount;
            int weaponHeight = level.weapon.frameHeight;

            //SDL_Rect weaponSrc = {(currentFrame % level.weapon.frameCount) * weaponWidth, 0, weaponWidth, weaponHeight};
            SDL_Rect weaponSrc = {0, 0, weaponWidth, weaponHeight};
            SDL_Rect weaponDest = {
                    towerDest.x + (towerDest.w - weaponWidth) / 2,
                    towerDest.y,  // Chống lên tower
                    weaponWidth, weaponHeight
            };

            SDL_RenderCopy(renderer, level.weapon.texture, &weaponSrc, &weaponDest);
        }
    }
}

void Play::render() {

    int timePlayed =  int ((SDL_GetTicks() - startTick) / 1000);

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

//    // Test vẻ enemy
//    Enemy enemy;
//    int enemyLevel = 0;
//    enemy = game->enemyManager.enemies[0];
//    SDL_Rect enemySrc = {(currentFrame % enemy.levels[enemyLevel].frame) * enemy.frameWidth, 0, enemy.frameWidth, enemy.frameHeight};
//    SDL_Rect enemyDest = {5 * TILE_SIZE, 5 * TILE_SIZE, TILE_SIZE, TILE_SIZE};
//    // Quay 90 độ
//    SDL_RenderCopyEx(renderer, enemy.texture, &enemySrc, &enemyDest, 180, NULL, SDL_FLIP_NONE);

    // Spawn enemy
    spawnEnemy();
    moveEnemies();
    renderEnemies();


    // Vẽ các tower đã đặt
    movePlayTowers();
    renderPlayTowers();


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

    SDL_Rect moneyRect = {10, bottomPanelY + 45, textSurface2->w, textSurface2->h};
    SDL_RenderCopy(renderer, moneyTexture, NULL, &moneyRect);


    char livesText[20];
    snprintf(livesText, sizeof(livesText), u8"Mạng: %02d", lives);
    SDL_Surface* textSurface3 = TTF_RenderUTF8_Blended(gameFont, livesText, {255, 255, 255, 255});
    SDL_Texture* livesTexture = SDL_CreateTextureFromSurface(renderer, textSurface3);
    SDL_FreeSurface(textSurface3);

    SDL_Rect livesRect = {10, bottomPanelY + 90, textSurface3->w, textSurface3->h};
    SDL_RenderCopy(renderer, livesTexture, NULL, &livesRect);
}


void Play::update() {
    // Cập nhật game logic nếu cần
}

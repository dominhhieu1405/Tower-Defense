#include "Play.h"
#include "Tower.h"
#include "Enemy.h"
#include "playTower.h"
#include "LevelSelect.h"
#include "playBullet.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <cmath>

Play::Play(SDL_Renderer* renderer, bool* isRunning, Game* game)
        : renderer(renderer), isRunning(isRunning), game(game), tilesetTexture(nullptr), buttonsTexture(nullptr) {
    // Lưu data
    std::ifstream file(game->dataPath);
    nlohmann::json jsonData;
    file >> jsonData;
    jsonData["selectedLevel"] = game->selectedLevel;
    file.close();
    std::ofstream outFile(game->dataPath);
    if (outFile.is_open()) {
        SDL_Log("Đã lưu dữ liệu vào file: %s", game->dataPath.c_str());
        outFile << jsonData.dump(4); // Làm đẹp JSON với 4 khoảng trắng mỗi tab
        outFile.close();
    } else {
        SDL_Log("Không thể mở file để ghi: %s", game->dataPath.c_str());
    }


    loadMap(game->selectedLevel);
    startTick = SDL_GetTicks();
    timeTick = SDL_GetTicks();
    lastFrameTime = SDL_GetTicks();
    currentFrame = 0;
    money = 500;
    lives = 5;
    playing = true;
    totalEnemies = levelData["total"];

    gameFont = TTF_OpenFont("assets/fonts/wood.ttf", 24); // Kích thước 24px
    if (!gameFont) {
        printf("Failed to load font: %s\n", TTF_GetError());
    }

    // Load logo
    logoTexture = IMG_LoadTexture(renderer, "assets/images/logo.png");
    if (!logoTexture) {
        SDL_Log("Failed to load logo: %s", SDL_GetError());
    }

    placeSound = Mix_LoadWAV("assets/audios/thud.wav");
    if (!placeSound) {
        SDL_Log("Failed to load click sound: %s", Mix_GetError());
    }
    clickSound = Mix_LoadWAV("assets/audios/click.wav");
    if (!clickSound) {
        SDL_Log("Failed to load click sound: %s", Mix_GetError());
    }

    buttonsTexture = loadTexture("assets/images/Buttons/Brown_Buttons_Pixel.png");
    Mix_VolumeMusic(30);
}

Play::~Play() {
    if (tilesetTexture) {
        SDL_DestroyTexture(tilesetTexture);
    }
    if (buttonsTexture) {
        SDL_DestroyTexture(buttonsTexture);
    }
    SDL_DestroyTexture(logoTexture);

    Mix_FreeChunk(placeSound);
    Mix_FreeChunk(clickSound);
    if (tempSound) {
        Mix_FreeChunk(tempSound);
    }
    Mix_CloseAudio();
    Mix_VolumeMusic(MIX_MAX_VOLUME);
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


                if (playing) {
                    /**
                     * Kiếm tra xem có kéo thả tower từ panel k
                     */

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


                    /**
                     * Kiểm tra nhấn nút menu
                     */
                    if (clickX >= 1200 && clickX <= 1248 && clickY >= 16 && clickY <= 64) {
                        playing = false;
                        stopTick = SDL_GetTicks();
                        SDL_Log("Tạm dừng game: %d", stopTick);
                    }

                    /**
                     * Kiểm tra nhấn vào tower đã đặt
                     */
                    if (!selectedTower) {
                        for (int i = 0; i < (int) towers.size(); i++) {
                            playTower &tower = towers[i];
                            int towerX = tower.x * TILE_SIZE;
                            int towerY = tower.y * TILE_SIZE;
                            if (clickX >= towerX && clickX <= towerX + TILE_SIZE &&
                                clickY >= towerY && clickY <= towerY + TILE_SIZE) {
                                // Chọn tower
                                selectedTower = &tower;
                                SDL_Log("Selected Tower: %d", i);
                                break;
                            }
                        }
                    } else {
                        hoverButton = -1;
                        if (clickX >= selectedTower->x * TILE_SIZE - 16 && clickX <= selectedTower->x * TILE_SIZE + 24 &&
                            clickY >= selectedTower->y * TILE_SIZE + TILE_SIZE - 16 && clickY <= selectedTower->y * TILE_SIZE + TILE_SIZE + 24) {
                            hoverButton = 1;
                            // Bán tower
                            money += selectedTower->tower->cost[selectedTower->level] / 2;
                            towers.erase(towers.begin() + (selectedTower - &towers[0]));
                            selectedTower = nullptr;
                            Mix_PlayChannel(-1, clickSound, 0);
                            SDL_Log("Bán Tower");
                        } else if (clickX >= selectedTower->x * TILE_SIZE + 40 && clickX <= selectedTower->x * TILE_SIZE + 80 &&
                            clickY >= selectedTower->y * TILE_SIZE + TILE_SIZE - 16 && clickY <= selectedTower->y * TILE_SIZE + TILE_SIZE + 24) {
                            SDL_Log("Nhấn vào nút nâng cấp tower");
                            // Nâng cấp tower
                            if (selectedTower->level < 2) {
                                // Kiểm tra xem có đủ tiền nâng cấp không
                                if (money < selectedTower->tower->cost[selectedTower->level + 1]) {
                                    SDL_Log("Không đủ tiền nâng cấp tower!");
                                    break;
                                }
                                selectedTower->level = selectedTower->level + 1;
                                SDL_Log("Nâng cấp tower lên level %d", selectedTower->level + 1);
                                // Trừ tiền
                                money -= selectedTower->tower->cost[selectedTower->level];
                            } else {
                                SDL_Log("Đã đạt cấp độ tối đa!");
                            }
                            hoverButton = 2;
                        } else {
                            // Bỏ chọn tower
                            selectedTower = nullptr;
                            SDL_Log("Bỏ chọn Tower");
                        }
                    }
                } else {
                    if (endTick < startTick) {
                        if (clickX >= (1280 - 240)/2 && clickX <= (1280 + 240)/2 &&
                            clickY >= 300 && clickY <= 380) {
                            Mix_PlayChannel(-1, clickSound, 0);
                            SDL_Log("Tiếp tục game: %d", SDL_GetTicks());
                            startTick = SDL_GetTicks() - stopTick + startTick;
                            stopTick = 0;
                            playing = true;
                            hoverButton = 1;

                        } else if (clickX >= (1280 - 240)/2 && clickX <= (1280 + 240)/2 &&
                                   clickY >= 390 && clickY <= 470) { // Chơi lại màn
                            Mix_PlayChannel(-1, clickSound, 0);
                            //hoverButton = 2;
                            // Reset dữ liệu màn chơi
                            towers.clear();
                            bullets.clear();
                            enemies.clear();
                            spawned.clear();
                            aliveEnemies.clear();
                            failedEnemies.clear();
                            killedEnemies.clear();
                            endTick = 0;
                            startTick = SDL_GetTicks();
                            soundStep = 0;
                            lastFrameTime = 0;
                            currentFrame = 0;
                            hoverButton = -1;
                            playing = true;
                            stopTick = 0;
                            lives = 5;
                            money = 500;

                            SDL_Log("Chơi lại màn chơi!");


                        } else if (clickX >= (1280 - 160)/2 && clickX <= (1280 + 160)/2 &&
                                   clickY >= 480 && clickY <= 560) { // Menu
                            Mix_PlayChannel(-1, clickSound, 0);
                            playing = true;
                            SDL_Log("Quay về menu: %d", SDL_GetTicks());
                            game->play = nullptr; // Giải phóng bộ nhớ
                            game->menu = nullptr; // Giải phóng bộ nhớ
                            game->selectedLevel = -1;
                            game->currentState = MENU;
                        }
                    } else {

                        if (clickX >= 480 && clickX <= 544 &&
                            clickY >= 576 && clickY <= 640) {
                            hoverButton = 1;

                            Mix_PlayChannel(-1, clickSound, 0);
                            playing = true;
                            SDL_Log("Quay về menu: %d", SDL_GetTicks());
                            game->play = nullptr; // Giải phóng bộ nhớ
                            game->menu = nullptr; // Giải phóng bộ nhớ
                            game->selectedLevel = -1;
                            game->currentState = MENU;

                        } else if (clickX >= 608 && clickX <= 672 &&
                                   clickY >= 576 && clickY <= 640) {
                            hoverButton = 2;
                            Mix_PlayChannel(-1, clickSound, 0);
                            //hoverButton = 2;
                            // Reset dữ liệu màn chơi
                            towers.clear();
                            bullets.clear();
                            enemies.clear();
                            spawned.clear();
                            aliveEnemies.clear();
                            failedEnemies.clear();
                            killedEnemies.clear();
                            endTick = 0;
                            startTick = SDL_GetTicks();
                            soundStep = 0;
                            lastFrameTime = 0;
                            currentFrame = 0;
                            hoverButton = -1;
                            playing = true;
                            stopTick = 0;
                            lives = 5;
                            money = 500;

                            SDL_Log("Chơi lại màn chơi!");


                        } else if (clickX >= 736 && clickX <= 800 &&
                                   clickY >= 576 && clickY <= 640) {
                            hoverButton = 3;

                            Mix_PlayChannel(-1, clickSound, 0);
                            playing = true;
                            SDL_Log("Quay về menu: %d", SDL_GetTicks());
                            game->play = nullptr; // Giải phóng bộ nhớ
                            game->selectedLevel = -1;
                            game->currentState = LEVEL_SELECT;
                        }
                    }
                }

            }
            break;

        case SDL_MOUSEMOTION:
            if (isDragging && draggedTower && playing) {
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
            if (!playing){
                if (endTick == 0) {
                    hoverButton = -1;
                    int mouseX = event.motion.x;
                    int mouseY = event.motion.y;
                    if (mouseX >= (1280 - 240)/2 && mouseX <= (1280 + 240)/2 &&
                        mouseY >= 300 && mouseY <= 380) {
                        hoverButton = 1;
                    } else if (mouseX >= (1280 - 240)/2 && mouseX <= (1280 + 240)/2 &&
                               mouseY >= 390 && mouseY <= 470) {
                        hoverButton = 2;
                    } else if (mouseX >= (1280 - 160)/2 && mouseX <= (1280 + 160)/2 &&
                               mouseY >= 480 && mouseY <= 560) {
                        hoverButton = 3;
                    }
                } else {
                    hoverButton = -1;
                    int mouseX = event.motion.x;
                    int mouseY = event.motion.y;

                    if (mouseX >= 480 && mouseX <= 544 &&
                        mouseY >= 576 && mouseY <= 640) {
                        hoverButton = 1;
                    } else if (mouseX >= 608 && mouseX <= 672 &&
                               mouseY >= 576 && mouseY <= 640) {
                        hoverButton = 2;
                    } else if (mouseX >= 736 && mouseX <= 800 &&
                               mouseY >= 576 && mouseY <= 640) {
                        hoverButton = 3;
                    }
                }
            } else {
                int mouseX = event.motion.x;
                int mouseY = event.motion.y;
                hoverButton = -1;
                if (selectedTower) {
                    if (mouseX >= selectedTower->x * TILE_SIZE - 16 && mouseX <= selectedTower->x * TILE_SIZE + 24 &&
                        mouseY >= selectedTower->y * TILE_SIZE + TILE_SIZE - 16 && mouseY <= selectedTower->y * TILE_SIZE + TILE_SIZE + 24) {
                        hoverButton = 1;
                    } else if (mouseX >= selectedTower->x * TILE_SIZE + 40 && mouseX <= selectedTower->x * TILE_SIZE + 80 &&
                               mouseY >= selectedTower->y * TILE_SIZE + TILE_SIZE - 16 && mouseY <= selectedTower->y * TILE_SIZE + TILE_SIZE + 24) {
                        hoverButton = 2;
                    }
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
                            isDragging = false;
                            draggedTower = nullptr;
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
                        std::sort(towers.begin(), towers.end(), [](const playTower& a, const playTower& b) {
                            return a.y != b.y ? a.y < b.y : a.x < b.x;
                        });

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

void Play::updateScore() {
    // Tính điểm
    int score = 0;
    if (lives == 5) {
        score = 3;
    } else if (lives >= 3) {
        score = 2;
    } else {
        score = 1;
    }

    // Đọc JSON từ file
    std::ifstream file(game->levelFile);
    nlohmann::json jsonData;
    file >> jsonData;
    file.close();

    int levelId = game->selectedLevel;

    for (auto& level : jsonData) {
        if (level["id"] == levelId) {
            level["completed"] = true;
            level["played"] = true;
            // Cập nhật nếu điểm mới cao hơn
            int oldScore = level.value("score", 0);
            if (score > oldScore) {
                level["score"] = score;
            }
        }

        // Mở khóa màn tiếp theo nếu vừa hoàn thành màn hiện tại
        if (level["id"] == levelId + 1) {
            level["unlocked"] = true;
        }
    }

    // Ghi lại file đã cập nhật
    std::ofstream outFile(game->levelFile);
    outFile << jsonData.dump(4); // làm đẹp JSON với 4 khoảng trắng
    outFile.close();
    (game->levelSelect)->loadLevels(game->levelFile.c_str());
    SDL_Log("Đã cập nhật điểm số: %d", score);
}


void Play::spawnEnemy() {
    double timePlayed =  (double (SDL_GetTicks() - startTick) / 1000.0);

    if (spawned.size() < levelData["total"]) {
        for (auto& stage: levelData["stages"]) {
            double stageTime = stage["time"];
            if (stageTime <= timePlayed) {
                for (auto& enemyData: stage["enemies"]) {
                    int enemyID = enemyData["id"];
                    int enemyType = enemyData["enemy"];
                    int enemyLevel = enemyData["level"];
                    double spawnTime = enemyData["time"];
                    // Check nếu enemyID đã spawn thì skip
                    if (spawned.find(enemyID) == spawned.end()) {
                        if (stageTime + spawnTime <= timePlayed) {
                            // Spawn enemy
                            playEnemy newEnemy;
                            newEnemy.id = enemyID;
                            newEnemy.enemy = &(game->enemyManager.enemies.at(enemyType));
                            newEnemy.health = newEnemy.enemy->levels[enemyLevel].health;
                            newEnemy.maxHealth = newEnemy.enemy->levels[enemyLevel].health;
                            newEnemy.speed = newEnemy.enemy->levels[enemyLevel].speed;
                            //newEnemy.speed = 6;
                            SDL_Log("Speed: %f", newEnemy.speed);
                            newEnemy.reward = newEnemy.enemy->levels[enemyLevel].reward;
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
                            aliveEnemies.insert(enemyID);
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
    for (int i = 0; i < (int)enemies.size(); i++) {
        playEnemy &enemy = enemies[i];
        if (enemy.status != 1)
            continue;

        Uint32 now = SDL_GetTicks();

        // 1. Chuẩn hóa góc quay về [0, 360)
        enemy.rotation = ((enemy.rotation % 360) + 360) % 360;

        // 2. Kiểm tra xem enemy đã ở tile cuối của đường đi chưa
        int totalPath = levelData["paths"][enemy.path].size();
        if (enemy.pathIndex == totalPath - 1) {

            // Dựa vào ID của tile ô cuối để cho enemy ra khỏi màn hình (vượt qua ranh giới tile)
            int tileIndex = mapData[enemy.tileY][enemy.tileX];
            SDL_Log("Tile index: %d", tileIndex);

            if ((tileIndex == 87 && enemy.x <= (enemy.tileX - 1) * TILE_SIZE) ||
                (tileIndex == 72 && enemy.x >= (enemy.tileX + 1) * TILE_SIZE) ||
                (tileIndex == 70 && enemy.y <= (enemy.tileY - 1) * TILE_SIZE) ||
                (tileIndex == 89 && enemy.y >= (enemy.tileY + 1) * TILE_SIZE)) {
                enemy.status = -1;
                failedEnemies.insert(enemy.id);
                aliveEnemies.erase(enemy.id);
                lives--;
            } else {
                int moveSpeed = (int)(enemy.speed * 4.0);
                if (tileIndex == 87) {
                    enemy.rotation = 90;
                    enemy.x -= moveSpeed;
                } else if (tileIndex == 72) {
                    enemy.rotation = 270;
                    enemy.x += moveSpeed;
                } else if (tileIndex == 70) {
                    enemy.rotation = 180;
                    enemy.y -= moveSpeed;
                } else if (tileIndex == 89) {
                    enemy.rotation = 0;
                    enemy.y += moveSpeed;
                }
            }
            continue;
        }

        // 3. Lấy tọa độ tile tiếp theo từ dữ liệu level
        int nextTileX = levelData["paths"][enemy.path][enemy.pathIndex + 1]["x"];
        int nextTileY = levelData["paths"][enemy.path][enemy.pathIndex + 1]["y"];
        int nextPixelX = nextTileX * TILE_SIZE;
        int nextPixelY = nextTileY * TILE_SIZE;

        // 4. Xác định góc quay mong muốn dựa vào vị trí hiện tại và tile tiếp theo
        int nextRotation = 0;
        if (nextTileX < enemy.tileX)      // Di chuyển sang trái
            nextRotation = 90;
        else if (nextTileX > enemy.tileX) // Di chuyển sang phải
            nextRotation = 270;
        else if (nextTileY > enemy.tileY) // Di chuyển xuống dưới
            nextRotation = 0;
        else if (nextTileY < enemy.tileY) // Di chuyển lên trên
            nextRotation = 180;

        // 5. Xử lý quay: Nếu enemy chưa đúng hướng thì quay từng bước 9 độ
        if (enemy.rotation != nextRotation) {
            // Chỉ thực hiện quay sau mỗi 30ms
            if (now - enemy.lastMove >= 30) {
                // Tính hiệu lệch góc cần quay, hiệu chuẩn cho hiệu lệch trong khoảng (-180, 180]
                int delta = nextRotation - enemy.rotation;
                if (delta > 180)  delta -= 360;
                if (delta <= -180) delta += 360;

                // Nếu lệch nhỏ hơn hoặc bằng 9 độ thì gán luôn
                if (abs(delta) <= 9)
                    enemy.rotation = nextRotation;
                else if (delta > 0)
                    enemy.rotation += 9;
                else
                    enemy.rotation -= 9;

                // Sau khi quay, cập nhật lại thời gian di chuyển
                enemy.lastMove = now;
            }
        }
        else {
            // 6. Nếu đã quay đúng hướng, tiến hành di chuyển theo hướng đó.
            // Chỉ di chuyển sau mỗi 62ms (theo tốc độ quy định)
            if (now - enemy.lastMove >= 62) {
                enemy.lastMove = now;
                int moveSpeed = (int)(enemy.speed * 4.0);

                switch (enemy.rotation) {
                    case 0:   enemy.y += moveSpeed; break;  // Xuống dưới
                    case 90:  enemy.x -= moveSpeed; break;  // Sang trái
                    case 180: enemy.y -= moveSpeed; break;  // Lên trên
                    case 270: enemy.x += moveSpeed; break;  // Sang phải
                }

                // 7. Kiểm tra nếu enemy đã đến hoặc vượt qua tâm ô tile tiếp theo.
                //    Dựa vào hướng di chuyển, so sánh vị trí hiện tại với tọa độ tile tiếp theo.
                if ((enemy.rotation == 0   && enemy.y >= nextPixelY) ||
                    (enemy.rotation == 180 && enemy.y <= nextPixelY) ||
                    (enemy.rotation == 90  && enemy.x <= nextPixelX) ||
                    (enemy.rotation == 270 && enemy.x >= nextPixelX)) {

                    // Căn chỉnh vị trí về tâm tile tiếp theo
                    enemy.x = nextPixelX;
                    enemy.y = nextPixelY;
                    enemy.tileX = nextTileX;
                    enemy.tileY = nextTileY;
                    enemy.pathIndex++;  // Chuyển sang tile tiếp theo trong path
                }
            }
        }
    }
}



//void Play::renderEnemies() {
//    for (int i = 0; i < (int) enemies.size(); i++) {
//        playEnemy& enemy = enemies[i];
//        if (enemy.status == 1) {
//            int enemyCurrentFrame = (SDL_GetTicks() - enemy.spawnTime) / 125;
//
//            SDL_Rect enemySrc = {(enemyCurrentFrame % enemy.enemy->levels[enemy.level].frame) * enemy.enemy->frameWidth, 3 * enemy.level * enemy.enemy->frameHeight, enemy.enemy->frameWidth, enemy.enemy->frameHeight};
//            SDL_Rect enemyDest = {enemy.x - 64, enemy.y, TILE_SIZE, TILE_SIZE};
//            // Quay 90 độ
//            SDL_RenderCopyEx(renderer, enemy.enemy->texture, &enemySrc, &enemyDest, enemy.rotation, NULL, SDL_FLIP_NONE);
//        }
//    }
//}

void Play::renderEnemies() {
    for (int i = 0; i < (int)enemies.size(); i++) {
        playEnemy& enemy = enemies[i];
        if (enemy.status == 1) {
            // Render enemy (sprite)
            int enemyCurrentFrame = (SDL_GetTicks() - enemy.spawnTime) / 125;
            SDL_Rect enemySrc = {
                    (enemyCurrentFrame % enemy.enemy->levels[enemy.level].frame) * enemy.enemy->frameWidth,
                    3 * enemy.level * enemy.enemy->frameHeight,
                    enemy.enemy->frameWidth,
                    enemy.enemy->frameHeight
            };
            SDL_Rect enemyDest = {
                    enemy.x,  // điều chỉnh theo vị trí của enemy
                    enemy.y,
                    TILE_SIZE,
                    TILE_SIZE
            };
            SDL_RenderCopyEx(renderer, enemy.enemy->texture, &enemySrc, &enemyDest, enemy.rotation, NULL, SDL_FLIP_NONE);

            // --- Render Thanh máu của enemy ---
            // Vị trí thanh máu: đặt trên enemy, chẳng hạn 10px phía trên vị trí y của enemyDest
            int barTotalWidth = 52;  // gồm 50px thanh máu + 1px viền trái và phải
            int barTotalHeight = 9;  // gồm 7px thanh máu + 1px viền trên và dưới
            int barX = enemy.x + (TILE_SIZE - barTotalWidth) / 2; // căn giữa trên sprite enemy
            int barY = enemy.y - 10; // 10px phía trên enemy

            // Vẽ viền thanh máu
            SDL_Rect borderRect = { barX, barY, barTotalWidth, barTotalHeight };
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // viền trắng
            SDL_RenderDrawRect(renderer, &borderRect);

            // Thanh máu bên trong (chiều rộng 50px, chiều cao 7px)
            SDL_Rect innerBar = { barX + 1, barY + 1, 50, 7 };
            // Nền của thanh máu (màu đen)
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer, &innerBar);

            // Tính phần đã đầy dựa trên tỉ lệ máu còn lại
            float healthPercent = (float)enemy.health / enemy.maxHealth;
            int fillWidth = (int)(50 * healthPercent);

            // Chọn màu theo tỉ lệ:
            // 1-20%: đỏ, 21-50%: vàng, 51-100%: xanh
            if (healthPercent <= 0.20f)
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            else if (healthPercent <= 0.50f)
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            else
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

            SDL_Rect fillRect = innerBar;
            fillRect.w = fillWidth;
            SDL_RenderFillRect(renderer, &fillRect);
            // --- End render Thanh máu ---
        }
    }
}


void Play::renderPlayTowers() {
    for (const auto& tower : towers) {
        SDL_Rect towerSrc = {64 * tower.level, 0, 64, 128}; // Lấy frame theo level
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

            SDL_Rect weaponSrc =  {0, 0, weaponWidth, weaponHeight};
            if (playing && tower.aimEnemy > -1) {
                weaponSrc.x = (currentFrame % level.weapon.frameCount) * weaponWidth;
            }
            //SDL_Rect weaponSrc = {(currentFrame % level.weapon.frameCount) * weaponWidth, 0, weaponWidth, weaponHeight};
            //SDL_Rect weaponSrc = {0, 0, weaponWidth, weaponHeight};
            SDL_Rect weaponDest = {
                    towerDest.x + (towerDest.w - weaponWidth) / 2,
                    towerDest.y - 9*tower.level,  // Chống lên tower
                    weaponWidth, weaponHeight
            };

//            SDL_RenderCopy(renderer, level.weapon.texture, &weaponSrc, &weaponDest);
            SDL_RenderCopyEx(renderer, level.weapon.texture, &weaponSrc, &weaponDest, tower.rotation + 90, NULL, SDL_FLIP_NONE);

        }
    }
}

void Play::movePlayTowers() {
    // Duyệt qua từng tower đã được đặt
    for (auto &tower : towers) {
        // Mặc định: chưa nhắm được enemy nào
        tower.aimEnemy = -1;

        // Tính tọa độ tâm của tower (chuyển từ tile sang pixel)
        int towerCenterX = tower.x * TILE_SIZE + TILE_SIZE / 2;
        int towerCenterY = tower.y * TILE_SIZE + TILE_SIZE / 2;


        // Lấy tầm bắn của tower dựa theo level (đơn vị pixel)
        int attackRange = tower.tower->attackRange[tower.level];
        int attackRangeSquared = attackRange * attackRange;

        // Vòng for duyệt qua enemy để tìm enemy đầu tiên trong tầm bắn
        for (int j = 0; j < (int)enemies.size(); j++) {
            playEnemy &enemy = enemies[j];

            // Chỉ xét enemy đang sống
            if (enemy.status != 1)
                continue;

            // tính tâm enemy bằng cách cộng thêm nửa TILE_SIZE cho x và y
            int enemyCenterX = enemy.x + TILE_SIZE / 2;
            int enemyCenterY = enemy.y + TILE_SIZE / 2;

            int dx = enemyCenterX - towerCenterX;
            int dy = enemyCenterY - towerCenterY;
            int distanceSquared = dx * dx + dy * dy;

            // Nếu enemy trong tầm bắn thì cập nhật aimEnemy và break
            if (distanceSquared <= attackRangeSquared) {
                tower.aimEnemy = j;
                //SDL_Log("Tower at (%d,%d) aimed at enemy (%d,%d) distance %d", towerCenterX, towerCenterY, enemyCenterX, enemyCenterY,(distanceSquared));
                break;
            }
        }

        // Nếu tower đã có enemy mục tiêu, tiến hành quay weapon theo enemy đó
        if (tower.aimEnemy != -1 && tower.aimEnemy < (int)enemies.size()) {
            playEnemy &target = enemies[tower.aimEnemy];
            int enemyCenterX = target.x + TILE_SIZE / 2;
            int enemyCenterY = target.y + TILE_SIZE / 2;

            // Tính góc mong muốn sử dụng atan2, trả về kết quả theo radian
            double deltaX = enemyCenterX - towerCenterX;
            double deltaY = enemyCenterY - towerCenterY;
            double targetAngleRad = atan2(deltaY, deltaX);
            int targetAngleDeg = static_cast<int>(round(targetAngleRad * 180.0 / M_PI)) % 360;
            if (targetAngleDeg < 0)
                targetAngleDeg += 360;

            // Cập nhật góc quay của tower theo cách mượt (bước quay 5 độ)
            int currentRotation = tower.rotation;
            int diff = targetAngleDeg - currentRotation;
            // Điều chỉnh hiệu chênh lệch để tìm góc ngắn nhất (trong khoảng -180 -> 180)
            if (diff > 180)
                diff -= 360;
            if (diff < -180)
                diff += 360;

            int rotationStep = 5; // Bước quay mỗi lần cập nhật
            if (abs(diff) <= rotationStep)
                tower.rotation = targetAngleDeg;
            else if (diff > 0)
                tower.rotation += rotationStep;
            else
                tower.rotation -= rotationStep;

            // Chuẩn hóa góc quay về [0, 360)
            tower.rotation = ((tower.rotation % 360) + 360) % 360;
            //SDL_Log("Tower at (%d,%d) rotation: %d", tower.x, tower.y, tower.rotation);
        }
        // Nếu không có enemy mục tiêu, giữ nguyên hướng hiện tại (không làm gì)
    }
}

void Play::attackEnemies() {
    Uint32 now = SDL_GetTicks();

    // Duyệt qua danh sách các tower (playTower)
    for (int i = 0; i < (int)towers.size(); i++) {
        playTower &tower = towers[i];

        // Nếu tower không có enemy mục tiêu (aimEnemy == -1) thì bỏ qua
        if (tower.aimEnemy == -1)
            continue;

        // Tính cooldown dựa theo tốc độ tấn công của tower
        // attackSpeed là số lần tấn công/giây → cooldown (ms) = 1000 / attackSpeed
        float atkSpeed = tower.tower->attackSpeed[tower.level];
        Uint32 cooldown = (Uint32)(1000.0f / atkSpeed);
        if (now - tower.lastAttackTime < cooldown)
            continue;

        // Cập nhật thời gian tấn công cuối của tower
        tower.lastAttackTime = now;

        // Tính tọa độ "đỉnh" của tháp (đi từ vị trí tháp)
        int towerCenterX = tower.x * TILE_SIZE + TILE_SIZE / 2;
        int towerTopY = tower.y * TILE_SIZE - 32;  // Đỉnh của tháp (vì tháp được vẽ với y - 32)

        // Lấy enemy mục tiêu. Kiểm tra chỉ số an toàn:
        if (tower.aimEnemy >= (int)enemies.size())
            continue;
        playEnemy &targetEnemy = enemies[tower.aimEnemy];

        // Tọa độ tâm của enemy (giả sử enemy.x, enemy.y là tọa độ góc trên bên trái)
        int enemyCenterX = targetEnemy.x + TILE_SIZE / 2;
        int enemyCenterY = targetEnemy.y + TILE_SIZE / 2;

        // Tạo viên đạn mới (Bullet)
        Bullet newBullet;
        newBullet.towerIndex = i;
        newBullet.enemyIndex = tower.aimEnemy;
        newBullet.startX = (float)towerCenterX;
        newBullet.startY = (float)towerTopY; // Dùng "đỉnh" của tháp
        newBullet.targetX = (float)enemyCenterX;
        newBullet.targetY = (float)enemyCenterY;
        newBullet.spawnTime = now;
        newBullet.duration = 500; // Ví dụ: 0.1 giây = 100ms

        // Lấy thông tin hoạt ảnh đạn từ towerLevel.projectile
        TowerLevel &lvl = tower.tower->levels[tower.level];
        newBullet.texture = lvl.projectile.texture;
        newBullet.frameCount = lvl.projectile.frameCount;
        newBullet.frameWidth = lvl.projectile.frameWidth / lvl.projectile.frameCount;
        newBullet.frameHeight = lvl.projectile.frameHeight;
        newBullet.rotation = tower.rotation; // Giữ nguyên góc quay của tháp

        // Thêm viên đạn mới vào danh sách
        bullets.push_back(newBullet);

        SDL_Log("Tower tại tile (%d, %d) bắn trúng enemy index %d", tower.x, tower.y, tower.aimEnemy);
    }
}
void Play::updateBullets() {
    Uint32 now = SDL_GetTicks();

    // Duyệt qua tất cả các viên đạn hiện có (với vòng lặp "while" để xoá phần tử dễ dàng)
    for (int i = 0; i < (int)bullets.size(); /* không tăng i ở đây */) {
        Bullet &bullet = bullets[i];
        // Tính tỉ lệ thời gian trôi qua (0.0 đến 1.0)
        float t = (now - bullet.spawnTime) / (float)bullet.duration;

        if (t >= 1.0f) {
            // Khi viên đạn đã bay đủ thời gian tức về đích, xử lý va chạm
            int towerIdx = bullet.towerIndex;
            int enemyIdx = bullet.enemyIndex;
            if (towerIdx < (int)towers.size() && enemyIdx < (int)enemies.size()) {
                playTower &tower = towers[towerIdx];
                playEnemy &targetEnemy = enemies[enemyIdx];

                // Tính sát thương theo damage của tower (theo cấp hiện tại)
                int damage = tower.tower->damage[tower.level];
                targetEnemy.health -= damage;
                if (targetEnemy.health <= 0 && aliveEnemies.find(targetEnemy.id) != aliveEnemies.end()) {
                    targetEnemy.status = 0;  // Enemy bị tiêu diệt
                    killedEnemies.insert(targetEnemy.id);
                    aliveEnemies.erase(targetEnemy.id);
                    // Công thêm tiền thưởng
                    money += targetEnemy.reward;
                    SDL_Log("Enemy index %d bị tiêu diệt bởi tower tại tile (%d, %d).", enemyIdx, tower.x, tower.y);

                    if (killedEnemies.size() == 1) {
                        tempSound = Mix_LoadWAV("assets/audios/FirstBlood.wav");
                        if (!tempSound) {
                            SDL_Log("Lỗi load âm thanh: %s", Mix_GetError());
                        } else {
                            Mix_VolumeChunk(tempSound, 128);
                            Mix_PlayChannel(-1, tempSound, 0);
                        }
                    } else if (aliveEnemies.size() == 0) {
                        tempSound = Mix_LoadWAV("assets/audios/QuetSach1.wav");
                        if (!tempSound) {
                            SDL_Log("Lỗi load âm thanh: %s", Mix_GetError());
                        } else {
                            Mix_VolumeChunk(tempSound, 128);
                            Mix_PlayChannel(-1, tempSound, 0);
                        }
                    }
                } else {
                    //SDL_Log("Enemy index %d mất %d máu, còn %d máu.", enemyIdx, damage, targetEnemy.health);
                }
            }
            // Xoá viên đạn vì nó đã trúng đích
            bullets.erase(bullets.begin() + i);
        } else {
            // Nếu đạn chưa hoàn thành hành trình, tăng i để xét viên đạn kế tiếp
            i++;
        }
    }
}
void Play::renderBullets() {
    Uint32 now = SDL_GetTicks();
    for (const Bullet &bullet : bullets) {
        float t = (now - bullet.spawnTime) / (float)bullet.duration;
        if (t > 1.0f) t = 1.0f; // Đảm bảo t không vượt quá 1

        // Tính vị trí hiện tại của viên đạn bằng cách nội suy tuyến tính
        int currentX = (int)(bullet.startX + t * (bullet.targetX - bullet.startX));
        int currentY = (int)(bullet.startY + t * (bullet.targetY - bullet.startY));

        // Tính frame của hoạt ảnh đạn. Giả sử toàn bộ hoạt ảnh diễn ra trong 100ms
        int currentFrame = (int)(t * bullet.frameCount) % bullet.frameCount;

        SDL_Rect srcRect = { currentFrame * bullet.frameWidth, 0, bullet.frameWidth, bullet.frameHeight };
        SDL_Rect destRect = { currentX - bullet.frameWidth/2, currentY - bullet.frameHeight/2, bullet.frameWidth, bullet.frameHeight };

        //SDL_RenderCopy(renderer, bullet.texture, &srcRect, &destRect);
        SDL_RenderCopyEx(renderer, bullet.texture, &srcRect, &destRect, bullet.rotation + 90, NULL, SDL_FLIP_NONE);
    }
}

void Play::renderDraggingTower(){
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

void Play::createRangeCircle(int radius) {
    int diameter = radius * 2;

    SDL_Texture* texture = SDL_CreateTexture(
            renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            diameter, diameter
    );

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // Clear trong suốt
    SDL_RenderClear(renderer);

    // Fill màu xanh lá nhạt (trong suốt)
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 64);
    for (int y = 0; y < diameter; ++y) {
        for (int x = 0; x < diameter; ++x) {
            int dx = x - radius;
            int dy = y - radius;
            if (dx * dx + dy * dy <= radius * radius) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }

    SDL_SetRenderTarget(renderer, NULL);

    rangeTexture = texture;
}

void Play::render() {

    if (playing && (totalEnemies == failedEnemies.size() + killedEnemies.size() || lives <= 0)) {
        endTick = SDL_GetTicks();
        playing = false;
    }

    int timePlayed =  int ((SDL_GetTicks() - startTick) / 1000);
    if (!playing) {
        timePlayed = int ((stopTick - startTick) / 1000);
    }
    if (endTick > 0) {
        timePlayed = int ((endTick - startTick) / 1000);
    }
    if (timePlayed == 0 && soundStep == 0){
        soundStep++;
        tempSound = Mix_LoadWAV("assets/audios/10s.wav");
        if (!tempSound) {
            SDL_Log("Lỗi load âm thanh: %s", Mix_GetError());
        } else {
            Mix_VolumeChunk(tempSound, 128);
            Mix_PlayChannel(-1, tempSound, 0);
        }
    }
    if (timePlayed == 9 && soundStep == 1) {
        soundStep++;
        tempSound = Mix_LoadWAV("assets/audios/3s.wav");
        if (!tempSound) {
            SDL_Log("Lỗi load âm thanh: %s", Mix_GetError());
        } else {
            Mix_VolumeChunk(tempSound, 128);
            Mix_PlayChannel(-1, tempSound, 0);
        }
    }

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

    // Render nếu đã chọn tower
    if (selectedTower) {
        int centerX = selectedTower->x * TILE_SIZE + TILE_SIZE / 2;
        int centerY = selectedTower->y * TILE_SIZE + TILE_SIZE / 2;
        int range = selectedTower->tower->attackRange[selectedTower->level];
        // Vẽ hình tròn tầm bắn
        if (!rangeTexture || currentRange != range) {
            if (rangeTexture) SDL_DestroyTexture(rangeTexture);
            createRangeCircle(range);
            currentRange = range;
        }

        SDL_Rect dst = { centerX - range, centerY - range, range * 2, range * 2 };
        SDL_RenderCopy(renderer, rangeTexture, NULL, &dst);
    }

    // Spawn enemy
    if (playing) {
        spawnEnemy();
        moveEnemies();
    }
    renderEnemies();


    // Vẽ các tower đã đặt
    if (playing) {
        movePlayTowers();
    }
    renderPlayTowers();
    if (playing) {
        attackEnemies();
        updateBullets();
    }
    renderBullets();

    if (selectedTower) { // Vẽ nút bán / nút nâng cấp
        SDL_Rect btnSrc = {48, 16, 16, 16};
        if (hoverButton == 1) {
            btnSrc.x += 160;
        }
        SDL_Rect btnDest = {selectedTower->x * TILE_SIZE - 16, selectedTower->y * TILE_SIZE + TILE_SIZE - 16, 40, 40};
        SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);

        // Nút nâng cấp
        btnSrc = {96, 16, 16, 16};
        if (hoverButton == 2) {
            btnSrc.x += 160;
        }
        btnDest.x += 56;
        SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);

    }


    // Vẽ bản sao Tower nếu đang kéo
    if (playing && isDragging && draggedTower) {
        renderDraggingTower();
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


    // Render các nút
    if (buttonsTexture){
        // Nút "Pause"
        SDL_Rect buttonSrc = {16*11, 16, 16, 16};
        SDL_Rect buttonDest = {1200, 16, 48, 48};
        SDL_RenderCopy(renderer, buttonsTexture, &buttonSrc, &buttonDest);
    }


    if (!playing && endTick == 0){
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 72);
        SDL_Rect highlightRect = {0, 0, TILE_SIZE*MAP_WIDTH, TILE_SIZE*MAP_HEIGHT};
        SDL_RenderFillRect(renderer, &highlightRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);


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

        /**
         * Render các nút trong menu
         */
        SDL_Rect btnSrc = {0, 16*5, 48, 16};
        SDL_Rect btnDest = {(1280 - 240)/2, 300, 240, 80};
        if (hoverButton == 1) {
            btnSrc.x = 160;
        }
        SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);

        btnSrc = {0, 16*7, 48, 16};
        if (hoverButton == 2) {
            btnSrc.x = 160;
        }
        btnDest = {(1280 - 240)/2, 390, 240, 80};
        SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);

        btnSrc = {16*2, 16*3, 32, 16};
        if (hoverButton == 3) {
            btnSrc.x = 192;
        }
        btnDest = {(1280 - 160)/2, 480, 160, 80};
        SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);
    } else if (endTick > 0) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 72);
        SDL_Rect highlightRect = {0, 0, 1280, 853};
        SDL_RenderFillRect(renderer, &highlightRect);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

        if (lives > 0) { // win
            // Hiện to ra trong 1s;
            int percent = int((SDL_GetTicks() - endTick) / 1000.0f * 100);
            if (percent > 100) {
                percent = 100;
            }
            int logoWidth = 744 * percent / 100;
            int logoHeight = 612 * percent / 100;
            SDL_Texture* tmpTexture = IMG_LoadTexture(renderer, "assets/images/victory.png");
            SDL_Rect logoRect = {(1280 - logoWidth) / 2,345 - 3*percent,logoWidth,logoHeight};
            SDL_RenderCopy(renderer, tmpTexture, NULL, &logoRect);
            SDL_DestroyTexture(tmpTexture);

            if (soundStep != -1 && (SDL_GetTicks() - endTick) > 500) {
                soundStep = -1;
                tempSound = Mix_LoadWAV("assets/audios/Victory.wav");
                updateScore();
                if (!tempSound) {
                    SDL_Log("Lỗi load âm thanh: %s", Mix_GetError());
                } else {
                    Mix_VolumeChunk(tempSound, 128);
                    Mix_PlayChannel(-1, tempSound, 0);
                }
            }
            if (SDL_GetTicks() - endTick > 1000) {
                SDL_Rect btnSrc = {16*3, 0, 16, 16};
                SDL_Rect btnDest = {640 - 160, 576, 64, 64};
                if (hoverButton == 1) {
                    btnSrc.x = 16*13;
                }
                SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);

                btnSrc = {16*9, 16*2, 16, 16};
                btnDest = {640 - 32, 576, 64, 64};
                if (hoverButton == 2) {
                    btnSrc.x = 16*19;
                }
                SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);

                btnSrc = {16*2, 16, 16, 16};
                btnDest = {640 + 96, 576, 64, 64};
                if (hoverButton == 3) {
                    btnSrc.x = 16*12;
                }
                SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);
            }
        } else {
            // Hiện to ra trong 1s;
            int percent = int((SDL_GetTicks() - endTick) / 1000.0f * 100);
            if (percent > 100) {
                percent = 100;
            }
            int logoWidth = 576 * percent / 100;
            int logoHeight = 639 * percent / 100;
            SDL_Texture* tmpTexture = IMG_LoadTexture(renderer, "assets/images/defeat.png");
            SDL_Rect logoRect = {(1280 - logoWidth) / 2,335 - 3*percent,logoWidth,logoHeight};
            SDL_RenderCopy(renderer, tmpTexture, NULL, &logoRect);
            SDL_DestroyTexture(tmpTexture);

            if (soundStep != -1 && (SDL_GetTicks() - endTick) > 500) {
                soundStep = -1;
                tempSound = Mix_LoadWAV("assets/audios/Defeat.wav");
                if (!tempSound) {
                    SDL_Log("Lỗi load âm thanh: %s", Mix_GetError());
                } else {
                    Mix_VolumeChunk(tempSound, 128);
                    Mix_PlayChannel(-1, tempSound, 0);
                }
            }
            if (SDL_GetTicks() - endTick > 1000) {
                SDL_Rect btnSrc = {16*3, 0, 16, 16};
                SDL_Rect btnDest = {640 - 160, 576, 64, 64};
                if (hoverButton == 1) {
                    btnSrc.x = 16*13;
                }
                SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);

                btnSrc = {16*9, 16*2, 16, 16};
                btnDest = {640 - 32, 576, 64, 64};
                if (hoverButton == 2) {
                    btnSrc.x = 16*19;
                }
                SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);

                btnSrc = {16*2, 16, 16, 16};
                btnDest = {640 + 96, 576, 64, 64};
                if (hoverButton == 3) {
                    btnSrc.x = 16*12;
                }
                SDL_RenderCopy(renderer, buttonsTexture, &btnSrc, &btnDest);
            }
        }
    }
}


void Play::update() {
    // Cập nhật game logic nếu cần
}

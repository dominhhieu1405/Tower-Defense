//
// Created by ADMIN on 19/04/2025.
//
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <filesystem>
#include <fstream>

std::string getUserDataPath() {
    std::string localPath = "assets/data/";
    std::error_code ec;

    // Tạo thư mục nếu chưa tồn tại
    std::filesystem::create_directories(localPath, ec);

    // Test quyền ghi
    std::string testFile = localPath + "test.tmp";
    std::ofstream testOut(testFile);
    if (testOut.is_open()) {
        testOut << "test";
        testOut.close();
        std::filesystem::remove(testFile, ec); // xóa file test
        return localPath;
    }

    // Nếu không ghi được thì dùng AppData
    char appdata[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, appdata))) {
        std::string fallbackPath = std::string(appdata) + "\\LuceVN\\TowerDefense\\";
        std::filesystem::create_directories(fallbackPath, ec);
        return fallbackPath;
    }

    // Nếu mọi cách đều thất bại thì dùng thư mục hiện tại
    return "./";
}

bool fileExists(const std::string& path) {
    return std::filesystem::exists(path);
}
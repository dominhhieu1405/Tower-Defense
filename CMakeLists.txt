cmake_minimum_required(VERSION 3.25)
project(tower_defense)

set(CMAKE_CXX_STANDARD 17)

# Thiết lập đường dẫn SDL2
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL2-2.32.0")
set(SDL2_MIXER_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL2_mixer-2.8.1")
set(SDL2_TTF_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL2_ttf-2.24.0")
set(SDL2_IMAGE_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL2_image-2.8.5")

# Tìm SDL2 & các thư viện
find_library(SDL2_LIB SDL2 PATHS ${SDL2_DIR}/lib)
find_library(SDL2_MAIN_LIB SDL2main PATHS ${SDL2_DIR}/lib)
find_library(SDL2_MIXER_LIB SDL2_mixer PATHS ${SDL2_MIXER_DIR}/lib)
find_library(SDL2_TTF_LIB SDL2_ttf PATHS ${SDL2_TTF_DIR}/lib)
find_library(SDL2_IMAGE_LIB SDL2_image PATHS ${SDL2_IMAGE_DIR}/lib)

# Kiểm tra lỗi nếu không tìm thấy thư viện
if (NOT SDL2_LIB OR NOT SDL2_MAIN_LIB OR NOT SDL2_MIXER_LIB OR NOT SDL2_TTF_LIB OR NOT SDL2_IMAGE_LIB)
        message(FATAL_ERROR "Không tìm thấy SDL2 hoặc các thư viện liên quan!")
endif()

# Đường dẫn file resource (icon)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.rc")


# Thêm thư mục include
include_directories(
        ${SDL2_DIR}/include
        ${SDL2_MIXER_DIR}/include
        ${SDL2_TTF_DIR}/include
        ${SDL2_IMAGE_DIR}/include
        src
)

# Tự động lấy tất cả file .cpp trong src/
file(GLOB SOURCES "src/*.cpp")


# Tạo file thực thi
# add_executable(tower_defense ${SOURCES} ${APP_ICON_RESOURCE_WINDOWS}) # Hiện cmd khi chạy
add_executable(tower_defense WIN32 ${SOURCES} ${APP_ICON_RESOURCE_WINDOWS}) # Không hiện cmd khi chạy

# Thêm thư viện , curl JSON
find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

find_package(CURL REQUIRED)
# Liên kết thư viện SDL2 và JSON
target_link_libraries(tower_defense
        ${SDL2_LIB}
        ${SDL2_MAIN_LIB}
        ${SDL2_MIXER_LIB}
        ${SDL2_TTF_LIB}
        ${SDL2_IMAGE_LIB}
        CURL::libcurl
        nlohmann_json::nlohmann_json
)

# Sao chép thư mục assets vào build/
add_custom_target(copy_resources ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets
        ${PROJECT_BINARY_DIR}/assets
        COMMENT "Copying resources into binary directory")
add_dependencies(tower_defense copy_resources)

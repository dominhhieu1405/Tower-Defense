cmake_minimum_required(VERSION 3.25)
project(tower_defense)

set(CMAKE_CXX_STANDARD 17)

set(SDL2_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL2-2.32.0")
set(SDL2_MIXER_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL2_mixer-2.8.1")
set(SDL2_TTF_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL2_ttf-2.24.0")
set(SDL2_IMAGE_DIR "${CMAKE_SOURCE_DIR}/SDL2/SDL2_image-2.8.5")

# Tìm SDL2
find_library(SDL2_LIB SDL2 PATHS ${SDL2_DIR}/lib)
find_library(SDL2_MAIN_LIB SDL2main PATHS ${SDL2_DIR}/lib)
find_library(SDL2_MIXER_LIB SDL2_mixer PATHS ${SDL2_MIXER_DIR}/lib)
find_library(SDL2_TTF_LIB SDL2_ttf PATHS ${SDL2_TTF_DIR}/lib)
find_library(SDL2_IMAGE_LIB SDL2_image PATHS ${SDL2_IMAGE_DIR}/lib)

# Thêm thư mục include
include_directories(
        ${SDL2_DIR}/include
        ${SDL2_MIXER_DIR}/include
        ${SDL2_TTF_DIR}/include
        ${SDL2_IMAGE_DIR}/include
)

add_executable(tower_defense main.cpp)

# Liên kết thư viện
target_link_libraries(tower_defense
        ${SDL2_LIB}
        ${SDL2_MAIN_LIB}
        ${SDL2_MIXER_LIB}
        ${SDL2_TTF_LIB}
        ${SDL2_IMAGE_LIB}
        )
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++")

add_custom_command(TARGET tower_defense POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/font.ttf
        ${CMAKE_BINARY_DIR}/font.ttf
        )

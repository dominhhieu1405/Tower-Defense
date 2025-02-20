#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>
#include "Game.h"

int main(int argc, char* argv[]) {
    Game game;

    if (!game.init("Tower Defense", 1200, 800)) {
        return -1;
    }

    game.run();
    game.cleanup();

    return 0;
}

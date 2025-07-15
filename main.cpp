#include <SDL2/SDL.h>
#include <iostream>
#include "settings.h"
#include "level.h"
#include "camera.h"
#include "player.h"
#include "weapon.h"
#include "ui.h"

class Game {
public:

    Game() {

        // SDL2 Boilerplate

        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL Could not initialize! SDL_Error:" << SDL_GetError() <<"\n";
            exit(1);
        }

        window = SDL_CreateWindow("Dōkutsu",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                WIDTH, HEIGHT,
                                SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "SDL Window could not be created! SDL_Error:" << SDL_GetError() << "\n";
            SDL_Quit();
            exit(1);
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            std::cerr << "SDL Renderer could not be created! SDL_Error:" << SDL_GetError() << "\n";
            SDL_DestroyWindow(window);
            SDL_Quit();
            exit(1);
        }

        // Level Initialization, Gameplay, Etc.
        level = std::make_unique<Level>(renderer);
    }

    ~Game() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

void run() {
    bool running = true;
    SDL_Event event;
    Uint32 frameDelay = 1000 / FPS;
    Uint32 frameStart;
    int frameTime;

    Camera camera(renderer, level->getVisibleSprites());
    UI ui(renderer, level->getPlayer());

        while (running) {
            frameStart = SDL_GetTicks();

            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                }
            }

            if (!level->getPlayer()->isAlive()) {
                std::cout << "Player has died. Ending game loop.\n";
                running = false;
                continue;
            }

            level->getPlayer()->handleInput();
            level->update();
            camera.centerOn(level->getPlayer()->getRect());

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            camera.draw();
            ui.update();
            ui.render();

            SDL_RenderPresent(renderer);

            frameTime = SDL_GetTicks() - frameStart;
            if (frameDelay > frameTime) {
                SDL_Delay(frameDelay - frameTime);
            }
        }

}


private:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::unique_ptr<Level> level;

};

int main(int argc, char* argv[]) {
    Game game;
    game.run();
    return 0;
}

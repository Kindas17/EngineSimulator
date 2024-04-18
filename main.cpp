#include <chrono>
#include <iostream>
#include <iomanip>
#include "Game.hpp"

#define FRAMETIME                      (20.0) /* ms */

using namespace std::chrono;

int main()
{
    printf("Program started\n");

    /* Game initialization */
    Game* game = new Game("Engine Simulator", 0, 0, 700, 700);
    microseconds frametime;

    printf("Game initialized\n");

    printf("Start the game loop\n");

    /* Game Loop */
    while (game->isGameRunning())
    {
        const int timeStart = SDL_GetTicks();
        const auto start = high_resolution_clock::now();
 
        system("clear");
        
        /* Rendering and information output */
        game->handleEvents();
        game->Render();

        // printf("Frametime: %f\n", frametime.count() / 1000.f);
        std::cout << std::fixed << std::setprecision(1) << 
        "Frametime: " << frametime.count() / 1000.f << " ms" << std::endl;
        
        /* Wait for next frame */
        int delay = FRAMETIME - (SDL_GetTicks() - timeStart);
        SDL_Delay((delay > 0) ? delay : 0);
        const auto end = high_resolution_clock::now();
        frametime = duration_cast<microseconds>(end - start);
    }

    game->Clean();
    return 0;
}


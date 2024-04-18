#include "Game.hpp"

Game::Game(const char* title, int xpos, int ypos, int height, int width) 
{
    SDL_Init(SDL_INIT_EVERYTHING);
    printf("SDL Initialized!\n");
    window = SDL_CreateWindow(title, xpos, ypos, width, height, 0);
    printf("Window created\n");
    renderer = SDL_CreateRenderer(window, -1, 0);
    printf("Renderer created\n");
    isRunning = true;
}

bool Game::isGameRunning()
{
    return isRunning;
}

void Game::handleEvents()
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_QUIT:
        isRunning = false;
        break;
    
    default:
        break;
    }
}

void Game::Render()
{
    static uint8_t cnt = 0;
    cnt++;
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderPresent(renderer);
}

void Game::Clean()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

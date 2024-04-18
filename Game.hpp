#ifndef GAME_HPP
#define GAME_HPP
#include <stdio.h>
#include <SDL2/SDL.h>
#include <string>

class Game
{
public:
    Game(const char* title, int xpos, int ypos, int height, int width);
    void handleEvents();
    void Render();
    bool isGameRunning();
    void Clean();

    bool isRunning;
    SDL_Window *window;
    SDL_Renderer *renderer;
private:
};

#endif

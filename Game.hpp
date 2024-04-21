#ifndef GAME_HPP
#define GAME_HPP
#include <stdio.h>
#include <SDL2/SDL.h>
#include <string>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

class Game
{
public:
    Game(const char* title, int xpos, int ypos, int height, int width);
    void handleEvents();
    void RenderClear();
    void RenderPresent();
    bool isGameRunning();
    void Clean();
    void QuitGame();

    bool isRunning;
    SDL_Window *window;
    SDL_Renderer *renderer;
private:
};

#endif

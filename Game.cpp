#include "Game.hpp"

Game::Game(const char *title, int xpos, int ypos, int height, int width) {
  SDL_Init(SDL_INIT_EVERYTHING);
  window = SDL_CreateWindow(title, xpos, ypos, width, height, 0);
  renderer = SDL_CreateRenderer(window, -1, 0);
  isRunning = true;
}

bool Game::isGameRunning() { return isRunning; }

void Game::handleEvents() {
  SDL_Event event;
  SDL_PollEvent(&event);

  switch (event.type) {
  case SDL_QUIT:
    isRunning = false;
    break;

  default:
    break;
  }

  ImGui_ImplSDL2_ProcessEvent(&event);
}

void Game::RenderClear() { SDL_RenderClear(renderer); }

void Game::RenderPresent() {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderPresent(renderer);
}

void Game::Clean() {
  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_Quit();
}

void Game::QuitGame() { isRunning = false; }

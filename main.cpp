#include "FrameRVis.hpp"
#include "Game.hpp"
#include "Logger.hpp"
#include "Piston.hpp"
#include "PistonGraphics.hpp"
#include <iomanip>
#include <iostream>

float FRAMETIME = 20.f; /* ms */
float pistonX = 350.f;
float pistonY = 500.f;
float engineSpeed = 0.f;

int main() {
  printf("Program started\n");

  /* Game initialization */
  Game *game = new Game("Engine Simulator", 0, 0, 700, 700);
  FrameRVis *fVis = new FrameRVis();
  CylinderGeometry *geom = new CylinderGeometry();
  Piston *piston = new Piston(*geom);
  CycleLogger *pistonPosLogger = new CycleLogger();
  CycleLogger *exhaustValveLog = new CycleLogger();
  CycleLogger *intakeValveLog = new CycleLogger();

  printf("Game initialized\n");

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSDL2_InitForSDLRenderer(game->window, game->renderer);
  ImGui_ImplSDLRenderer2_Init(game->renderer);

  printf("Start the game loop\n");

  /* Game Loop */
  while (game->isGameRunning()) {
    fVis->startClock();
    const int timeStart = SDL_GetTicks();
    PistonGraphics *pistonGraphics = new PistonGraphics(
        (vector2_T){.x = pistonX, .y = pistonY}, piston, 2000);

    /* Simulation */
    piston->updatePosition(FRAMETIME / 1000.f, engineSpeed);
    /* Log Data */
    pistonPosLogger->addSample(piston->getPistonPosition());
    exhaustValveLog->addSample(piston->exhaustValve);
    intakeValveLog->addSample(piston->intakeValve);
    if (piston->cycleTrigger) {
      pistonPosLogger->trig();
      exhaustValveLog->trig();
      intakeValveLog->trig();
      piston->cycleTrigger = false;
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(game->window);
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::Text("Framerate: %.2f fps", fVis->getFramerate());
    ImGui::InputFloat("Frametime [ms]", &FRAMETIME, 0, 0, "%.0f", 0);
    ImGui::PlotLines("Framerate [fps]", fVis->getData(), fVis->getSize());
    ImGui::InputFloat("Engine speed", &engineSpeed, 0, 0, "%.0f", 0);
    ImGui::PlotLines("Piston Position", pistonPosLogger->getData(),
                     pistonPosLogger->getSize());
    ImGui::PlotLines("Intake Valve", intakeValveLog->getData(),
                     intakeValveLog->getSize());
    ImGui::PlotLines("Exhaust Valve", exhaustValveLog->getData(),
                     exhaustValveLog->getSize());
    if (ImGui::Button("Quit")) {
      game->QuitGame();
    };
    ImGui::End();

    /* Rendering */
    ImGui::Render();
    game->handleEvents();
    game->RenderClear();

    pistonGraphics->showPiston(game->renderer);

    /* Wait for next frame */
    int delay = FRAMETIME - (SDL_GetTicks() - timeStart);
    SDL_Delay((delay > 0) ? delay : 0);

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    game->RenderPresent();
    fVis->endClock();
  }

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  game->Clean();
  return 0;
}

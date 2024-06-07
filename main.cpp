#include "FrameRVis.hpp"
#include "Game.hpp"
#include "Logger.hpp"
#include "Piston.hpp"
#include "PistonGraphics.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>

constexpr float getTimeStep_s(int mult, float frametime) {
  return frametime / (1000.f * mult);
}

constexpr int SIMULATION_MULTIPLIER = 50;
constexpr float FRAMETIME = 25.f; /* ms */
constexpr size_t SIZE_LOG = 2.f / (0.001f * FRAMETIME);

int main(int argc, char *argv[]) {

  bool start = false;
  size_t gameLoopCnt = 0;

  /* Game initialization */
  Game *game = new Game("Engine Simulator", SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED, 1000, 1000);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSDL2_InitForSDLRenderer(game->window, game->renderer);
  ImGui_ImplSDLRenderer2_Init(game->renderer);

  CylinderGeometry *geom = new CylinderGeometry();
  Piston *piston = new Piston(*geom);
  Logger *presLog = new Logger(3000);
  Logger *voluLog = new Logger(3000);
  Logger *nrLog = new Logger(3000);
  Logger *tempLog = new Logger(3000);

  /* Game Loop */
  while (game->isGameRunning()) {

    const int timeStart = SDL_GetTicks();

    PistonGraphics *pistonGraphics =
        new PistonGraphics(vector2_T{.x = 350.f, .y = 550.f}, piston, 2000);

    if (start) {
      gameLoopCnt++;

      // Simulation

      for (size_t i = 0; i < SIMULATION_MULTIPLIER; ++i) {

        const float deltaT = getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME);
        const float t = 0.001f * (gameLoopCnt * FRAMETIME +
                                  i * FRAMETIME / SIMULATION_MULTIPLIER);

        piston->update(deltaT);

        presLog->addSample(RADSToRPM(piston->state[1]));
        voluLog->addSample(M3ToCC(piston->getChamberVolume()));
        nrLog->addSample(M3ToCC(piston->gas->getV()));
        tempLog->addSample(piston->state[0]);

        // if (piston->cycleTrigger) {
        //   presLog->trig();
        //   voluLog->trig();
        //   nrLog->trig();
        //   tempLog->trig();
        //   piston->cycleTrigger = false;
        // }

        // End Simulation
      }
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::Text("Time:       %.1f s", 0.001f * gameLoopCnt * FRAMETIME);
    ImGui::Text("Framerate:  %.0f Hz", 1000.f / FRAMETIME);
    ImGui::Text("Simulation: %.0f Hz",
                SIMULATION_MULTIPLIER * 1000.f / FRAMETIME);
    ImGui::Text("Engine Speed:  %.0f rpm", RADSToRPM(piston->state[1]));
    ImGui::Checkbox("Start", &start);
    ImGui::SliderFloat("External Torque [Nm]", &piston->externalTorque, 0.f,
                       20.f);
    ImGui::SliderFloat("Intake Coef", &piston->intakeCoef, 0.000012f, 0.0012f,
                       "%.6f");
    ImGui::SliderFloat("Exhaust Coef", &piston->exhaustCoef, 0.000008f, 0.0008f,
                       "%.6f");
    ImGui::End();

    ImGui::Begin("Test4");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Speed [rpm]", presLog->getData(), presLog->getSize());
    ImPlot::EndPlot();
    ImGui::End();

    // ImGui::Begin("Test5");
    // ImPlot::SetNextAxesToFit();
    // ImPlot::BeginPlot("ASD");
    // ImPlot::PlotLine("Temperature [°C]", tempLog->getData(),
    //                  tempLog->getSize());
    // ImPlot::EndPlot();
    // ImGui::End();

    ImGui::Begin("Test6");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    // ImPlot::PlotLine("Thermodynamic Cycle", voluLog->getData(),
    //                  presLog->getData(), presLog->getSize());
    ImPlot::PlotLine("Volume Chamber", voluLog->getData(), voluLog->getSize());
    ImPlot::PlotLine("Volume Gas", nrLog->getData(), nrLog->getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Test7");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    // ImPlot::PlotLine("Current Angle", nrLog->getData(), nrLog->getSize());
    ImPlot::PlotLine("State[0]", tempLog->getData(), tempLog->getSize());
    ImPlot::EndPlot();
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
  }

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  game->Clean();
  return 0;
}

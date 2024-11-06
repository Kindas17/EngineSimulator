#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <thread>
#include <vector>

#include "FrameRVis.hpp"
#include "Game.hpp"
#include "Logger.hpp"
#include "Orifice.hpp"
#include "Piston.hpp"
#include "PistonGraphics.hpp"
#include "Solver.hpp"

constexpr float getTimeStep_s(int mult, float frametime) {
  return frametime / (1000.f * mult);
}

constexpr int SIMULATION_MULTIPLIER = 200;
constexpr float FRAMETIME = 16.f; /* ms */
constexpr size_t SIZE_LOG = 2.f / (0.001f * FRAMETIME);

constexpr float simulationFrequency() {
  return SIMULATION_MULTIPLIER * 1000.f / FRAMETIME;
}

float cpuLoad = 0.f;

int main(int argc, char *argv[]) {
  bool start = false;
  size_t gameLoopCnt = 0;

  /* Game initialization */
  Game game = Game("Engine Simulator",
                   SDL_WINDOWPOS_UNDEFINED,
                   SDL_WINDOWPOS_UNDEFINED,
                   1000,
                   1000);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSDL2_InitForSDLRenderer(game.window, game.renderer);
  ImGui_ImplSDLRenderer2_Init(game.renderer);

  // Initialize SDL_image (supports PNG, JPG, etc.)
  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    SDL_Log(
        "SDL_image could not initialize! SDL_image Error: %s", IMG_GetError());
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    SDL_Quit();
    return -1;
  }

  // Define the engine
  EngineConfig cfg;
  if (!cfg.loadFromFile("engine.json")) {
    std::cerr << "Game OVER!" << std::endl;
    return 0;
  };
  cfg.evaluate();
  Piston piston = Piston(cfg);
  PistonGraphics pistonGraphics =
      PistonGraphics(std::valarray<float>{350.f, 600.f}, &piston, 2000);

  // Define the loggers
  std::vector<CycleLogger> loggers = {
      CycleLogger([&piston]() { return PAToATM(piston.gas.getP()); }),
      CycleLogger([&piston]() { return piston.intakeFlow; }),
      CycleLogger([&piston]() { return KELVToCELS(piston.gas.getT()); }),
      CycleLogger([&piston]() { return piston.gas.getOx(); }),
      CycleLogger([&piston]() { return piston.exhaustFlow; }),
      CycleLogger([&piston]() { return piston.gas.getFuel(); }),
      CycleLogger([&piston]() { return M3ToCC(piston.gas.getV()); })};

  /* Game Loop */
  while (game.isGameRunning()) {
    const auto timeStart = high_resolution_clock::now();
    const float deltaT = getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME);

    if (start) {
      gameLoopCnt++;

      // Simulation
      for (size_t i = 0; i < SIMULATION_MULTIPLIER; ++i) {
        piston.update(deltaT);

        for (auto &logger : loggers) {
          logger.addSample();
        }

        if (piston.cycleTrigger) {
          for (auto &logger : loggers) {
            logger.trig();
          }
          piston.cycleTrigger = false;
        }
      }
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::SliderFloat("Torque [Nm]", &piston.externalTorque, 0.f, 20.f);
    ImGui::SliderFloat("Throttle", &piston.throttle, 0.f, 1.f);
    ImGui::InputFloat("Combustion speed", &piston.combustionSpeed);
    ImGui::InputFloat("Combustion energy", &piston.combustionEnergy);
    ImGui::End();

    ImGui::Begin("ASD 1");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Thermodynamic Cycle",
                     loggers[6].getData(),
                     loggers[0].getData(),
                     loggers[0].getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("ASD 2");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Intake flow", loggers[1].getData(), loggers[1].getSize());
    ImPlot::PlotLine(
        "Exhaust flow", loggers[4].getData(), loggers[4].getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("ASD 3");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Temperature", loggers[2].getData(), loggers[2].getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("ASD 4");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("O2", loggers[3].getData(), loggers[3].getSize());
    ImPlot::PlotLine("Fuel", loggers[5].getData(), loggers[5].getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Test8");
    ImGui::Text("Time:       %.1f s", 0.001f * gameLoopCnt * FRAMETIME);
    ImGui::Text("Framerate:  %.0f Hz", 1000.f / FRAMETIME);
    ImGui::Text("Simulation: %.0f Hz", simulationFrequency());
    ImGui::Text("Engine Speed:  %.0f rpm", RADSToRPM(piston.getEngineSpeed()));
    ImGui::Text("CPU Load:     %.0f / 100", 100 * cpuLoad);
    ImGui::Checkbox("Start", &start);
    ImGui::Checkbox("Ignition", &piston.ignitionOn);
    ImGui::End();

    /* Rendering */
    ImGui::Render();

    game.handleEvents();
    game.RenderClear();

    pistonGraphics.showPiston(game.renderer);

    /* Wait for next frame */
    const auto timeEnd = high_resolution_clock::now();
    const auto deltaTime =
        duration_cast<microseconds>(timeEnd - timeStart).count();
    const auto delay = FRAMETIME - (deltaTime / 1000);
    cpuLoad = 0.99f * cpuLoad + 0.01f * deltaTime / (FRAMETIME * 1000);
    std::this_thread::sleep_for(std::chrono::microseconds(
        static_cast<int>(FRAMETIME * 1000) - deltaTime));

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    game.RenderPresent();
  }

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  game.Clean();
  return 0;
}

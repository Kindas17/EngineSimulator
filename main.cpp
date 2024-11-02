#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>

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

  // Example gasses
  CylinderGeometry geometry = CylinderGeometry();
  IdealGas gas1 = IdealGas(
      1 * DEFAULT_AMBIENT_PRESSURE, 1.f, 4 * DEFAULT_AMBIENT_TEMPERATURE);
  IdealGas gas2 = IdealGas(
      1 * DEFAULT_AMBIENT_PRESSURE, 1.f, 1 * DEFAULT_AMBIENT_TEMPERATURE);
  Orifice orif1 = Orifice(0.01f, gas1, gas2);

  /* Game Loop */
  while (game.isGameRunning()) {
    const auto timeStart = high_resolution_clock::now();

    const float deltaT = getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME);

    if (start) {
      gameLoopCnt++;

      // Simulation
      for (size_t i = 0; i < SIMULATION_MULTIPLIER; ++i) {
        auto stp = orif1.flowThrough();
        gas1.state = RungeKutta4(
            deltaT,
            0.f,
            gas1.state,
            std::bind(
                F_IdealGas,
                std::placeholders::_1,
                std::placeholders::_2,
                +stp + gas1.exchangeHeat(1.f, DEFAULT_AMBIENT_TEMPERATURE)));
        gas2.state = RungeKutta4(
            deltaT,
            0.f,
            gas2.state,
            std::bind(
                F_IdealGas,
                std::placeholders::_1,
                std::placeholders::_2,
                -stp - gas1.exchangeHeat(1.f, DEFAULT_AMBIENT_TEMPERATURE)));

        std::cout << "Gas1 P: " << gas1.getP() << std::endl;
        std::cout << "Gas2 P: " << gas2.getP() << std::endl;
        std::cout << "Gas1 T: " << gas1.getT() << std::endl;
        std::cout << "Gas2 T: " << gas2.getT() << std::endl;
        std::cout << "---" << std::endl << std::endl;
      }
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // ImGui::Begin("Fuel Amount");
    // ImPlot::SetNextAxesToFit();
    // ImPlot::BeginPlot("ASD");
    // ImPlot::PlotLine("Fuel Amount", fuelLog.getData(), fuelLog.getSize());
    // ImPlot::EndPlot();
    // ImGui::End();

    ImGui::Begin("Test8");
    ImGui::Text("Time:       %.1f s", 0.001f * gameLoopCnt * FRAMETIME);
    ImGui::Text("Framerate:  %.0f Hz", 1000.f / FRAMETIME);
    ImGui::Text(
        "Simulation: %.0f Hz", SIMULATION_MULTIPLIER * 1000.f / FRAMETIME);
    ImGui::Text("CPU Load:     %.0f / 100", 100 * cpuLoad);

    ImGui::Checkbox("Start", &start);
    ImGui::End();

    /* Rendering */
    ImGui::Render();

    game.handleEvents();
    game.RenderClear();

    /* Wait for next frame */
    const auto timeEnd = high_resolution_clock::now();
    const auto deltaTime =
        duration_cast<microseconds>(timeEnd - timeStart).count();
    const auto delay = FRAMETIME - (deltaTime / 1000);
    cpuLoad = 0.99f * cpuLoad + 0.01f * deltaTime / (FRAMETIME * 1000);
    SDL_Delay((delay > 0) ? delay : 0);

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

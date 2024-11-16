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

constexpr int AUDIO_FREQ = 44100;
constexpr int SIMULATION_MULTIPLIER = 117;
constexpr float FRAMETIME = 16.f; /* ms */
constexpr size_t SIZE_LOG = 2.f / (0.001f * FRAMETIME);
constexpr float SIMULATION_FREQUENCY =
    SIMULATION_MULTIPLIER * 1000.f / FRAMETIME;
constexpr size_t AUDIO_SAMPLES =
    SIMULATION_MULTIPLIER * AUDIO_FREQ / SIMULATION_FREQUENCY;
constexpr size_t CIRCULAR_BUFFER_SIZE = AUDIO_SAMPLES * 5;
constexpr size_t RESAMPLING_FACTOR = AUDIO_SAMPLES / SIMULATION_MULTIPLIER;

constexpr size_t overhead = 0;
std::vector<float> resampledData(AUDIO_SAMPLES + overhead);

float cpuLoad = 0.f;

int main(int argc, char *argv[]) {
  SDL_Init(SDL_INIT_AUDIO);

  SDL_AudioSpec desiredSpec;
  SDL_zero(desiredSpec);
  desiredSpec.freq = AUDIO_FREQ;
  desiredSpec.format = AUDIO_F32SYS;
  desiredSpec.channels = 1;
  desiredSpec.samples = AUDIO_SAMPLES;

  // Open the audio device in non-callback mode (using queue).
  if (SDL_OpenAudio(&desiredSpec, nullptr) < 0) {
    std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  // Start audio playback.
  SDL_PauseAudio(0);

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
      CycleLogger([&piston]() { return M3ToCC(piston.gas.getV()); }),
      CycleLogger(
          [&piston]() { return PAToATM(piston.intakeManifold.getP()); }),
      CycleLogger([&piston]() { return PAToATM(piston.exhaustPipe.getP()); }),
      CycleLogger([&piston]() { return piston.intakeManifold.getOx(); }),
      CycleLogger([&piston]() { return piston.exhaustPipe.getOx(); }),
  };

  /* Game Loop */
  while (game.isGameRunning()) {
    const auto timeStart = high_resolution_clock::now();
    const float deltaT = getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME);

    if (start) {
      gameLoopCnt++;

      // Simulation
      for (size_t i = 0; i < SIMULATION_MULTIPLIER; ++i) {
        piston.update(deltaT);

        // Audio resampling
        const auto rem = AUDIO_SAMPLES -
                         RESAMPLING_FACTOR * SIMULATION_MULTIPLIER + overhead;
        for (size_t j = 0; j < RESAMPLING_FACTOR + rem; ++j) {
          resampledData[RESAMPLING_FACTOR * i + j] =
              ((PAToATM(piston.intakeManifold.getP()) - 1) +
               (PAToATM(piston.exhaustPipe.getP()) - 1));
        }

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
    ImGui::InputFloat("Combustion speed", &piston.cfg.combustion.speed);
    ImGui::InputFloat("Combustion energy", &piston.cfg.combustion.energy);
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
    ImPlot::PlotLine("Chm Ox", loggers[3].getData(), loggers[3].getSize());
    ImPlot::PlotLine("Int Ox", loggers[9].getData(), loggers[9].getSize());
    ImPlot::PlotLine("Exh Ox", loggers[10].getData(), loggers[10].getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("ASD 4");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine(
        "Intake Pressure", loggers[7].getData(), loggers[7].getSize());
    ImPlot::PlotLine(
        "Exhaust Pressure", loggers[8].getData(), loggers[8].getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Test8");
    ImGui::Text("Time:       %.1f s", 0.001f * gameLoopCnt * FRAMETIME);
    ImGui::Text("Framerate:  %.0f Hz", 1000.f / FRAMETIME);
    ImGui::Text("Simulation: %.0f Hz", SIMULATION_FREQUENCY);
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

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    game.RenderPresent();

    // Queue the samples into SDL’s audio buffer.
    SDL_QueueAudio(
        1, resampledData.data(), resampledData.size() * sizeof(float));

    /* Wait for next frame */
    const auto deltaTime =
        duration_cast<microseconds>(high_resolution_clock::now() - timeStart)
            .count();
    const auto delay = FRAMETIME - (deltaTime / 1000);
    cpuLoad = 0.99f * cpuLoad + 0.01f * deltaTime / (FRAMETIME * 1000);

    std::this_thread::sleep_for(std::chrono::microseconds(
        static_cast<int>(FRAMETIME * 1000) - deltaTime));
  }

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  SDL_CloseAudio();

  game.Clean();
  return 0;
}

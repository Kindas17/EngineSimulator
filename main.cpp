#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <semaphore>
#include <thread>
#include <unordered_map>
#include <vector>

#include "EngineControls.hpp"
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

constexpr int DESIRED_SIM_OVERHEAD = 3;
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
std::vector<float> resampledData(AUDIO_SAMPLES);

// Engine Controls
EngineControlsMgm engineControlsMgm;

float cpuLoad = 0.f;

int counter_gra = 0;
int counter_sim = 0;
bool simulation_go = true;
std::binary_semaphore t1_semaphore{0};
size_t piston_data_size = 10;
std::vector<std::valarray<float>> piston_data(piston_data_size);
size_t sim_idx = 0;
size_t gra_idx = 0;
int sim_overhead = 0;
int thread_multi = 1;

void simulation(EngineConfig const &cfg) {
  Piston piston = Piston(cfg);

  while (simulation_go) {
    // Update the engine controls
    auto ctrls = engineControlsMgm.getControls();
    piston.externalTorque = ctrls.externalTorque;
    piston.throttle = ctrls.throttle;
    piston.ignitionOn = ctrls.ignitionOn;

    while (thread_multi > 0) {
      counter_sim++;
      for (size_t i = 0; i < SIMULATION_MULTIPLIER; ++i) {
        piston.update(getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME));
      }

      piston_data[sim_idx] = std::valarray<float>{piston.getCurrentAngle(),
                                                  piston.getThetaAngle(),
                                                  piston.intakeValve,
                                                  piston.exhaustValve};
      sim_idx = (sim_idx + 1) % piston_data_size;

      thread_multi--;
    }

    // Update the engine state
    engineControlsMgm.setState(EngineState{piston.getEngineSpeed()});
    t1_semaphore.acquire();
  }
}

int main(int argc, char *argv[]) {
  // Define the engine
  EngineConfig cfg;
  if (!cfg.loadFromFile("engine.json")) {
    std::cerr << "Game OVER!" << std::endl;
    return 0;
  };
  cfg.evaluate();

  thread_multi = DESIRED_SIM_OVERHEAD;
  std::thread t1(simulation, cfg);

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

  EngineControls engCtrls{};
  EngineState engState{};

  /* Game Loop */
  while (game.isGameRunning()) {
    const auto timeStart = high_resolution_clock::now();
    const float deltaT = getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME);

    // Get the current engine state
    engState = engineControlsMgm.getState();

    counter_gra++;

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::SliderFloat("Torque [Nm]", &engCtrls.externalTorque, 0.f, 20.f);
    ImGui::SliderFloat("Throttle", &engCtrls.throttle, 0.f, 1.f);
    // ImGui::InputFloat("Combustion speed", &piston.cfg.combustion.speed);
    // ImGui::InputFloat("Combustion energy", &piston.cfg.combustion.energy);
    ImGui::End();

    ImGui::Begin("Test8");
    ImGui::Text("Time:       %.1f s", 0.001f * gameLoopCnt * FRAMETIME);
    ImGui::Text("Framerate:  %.0f Hz", 1000.f / FRAMETIME);
    ImGui::Text("Simulation: %.0f Hz", SIMULATION_FREQUENCY);
    ImGui::Text("Engine Speed:  %.0f rpm", RADSToRPM(engState.engineSpeed));
    ImGui::Text("CPU Load:     %.0f / 100", 100 * cpuLoad);
    ImGui::Text("Graphics  :     %d", counter_gra);
    ImGui::Text("Simulation:     %d", counter_sim);
    ImGui::Text("sim_overhead:   %d | %d",
                int(sim_overhead),
                counter_sim - counter_gra);

    // ImGui::Checkbox("Start", &start);
    ImGui::Checkbox("Ignition", &engCtrls.ignitionOn);
    ImGui::End();

    /* Rendering */
    ImGui::Render();

    game.handleEvents();
    game.RenderClear();

    PistonGraphics pistonGraphics = PistonGraphics(
        std::valarray<float>{350.f, 600.f}, piston_data[gra_idx], cfg, 2000);
    pistonGraphics.showPiston(game.renderer);

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    game.RenderPresent();

    // Update engine controls
    engineControlsMgm.setControls(engCtrls);

    // Prepare for next frame
    gra_idx = (gra_idx + 1) % piston_data_size;
    // Check simulation overhead
    sim_overhead = (sim_idx >= gra_idx)
                       ? sim_idx - gra_idx
                       : (sim_idx + piston_data_size) - gra_idx;
    // Release simulation thread
    thread_multi = (DESIRED_SIM_OVERHEAD + 1) - sim_overhead;
    t1_semaphore.release();

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

  simulation_go = false;
  t1_semaphore.release();
  t1.join();

  game.Clean();
  return 0;
}

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
#include "Game.hpp"
#include "Logger.hpp"
#include "Orifice.hpp"
#include "Piston.hpp"
#include "PistonGraphics.hpp"
#include "Solver.hpp"

// Function to linearly interpolate a signal from N samples to M samples
template <size_t M>
std::array<float, M> resampleLinear(const std::vector<float> &input) {
  size_t N = input.size();
  if (N == 0 || M == 0) {
    throw std::invalid_argument("Input signal and M must have non-zero size.");
  }

  std::array<float, M> output;

  // Calculate the resampling ratio
  float scale = static_cast<float>(N - 1) / (M - 1);

  for (size_t i = 0; i < M; ++i) {
    // Calculate the corresponding position in the input
    float pos = i * scale;
    size_t idx = static_cast<size_t>(pos);  // Integer part
    float frac = pos - idx;                 // Fractional part

    // Handle edge case for the last sample
    if (idx + 1 >= N) {
      output[i] = input[idx];
    } else {
      // Linear interpolation
      output[i] = input[idx] * (1.0 - frac) + input[idx + 1] * frac;
    }
  }

  return output;
}

constexpr float getTimeStep_s(int mult, float frametime) {
  return frametime / (1000.f * mult);
}

constexpr int DESIRED_SIM_OVERHEAD = 3;
constexpr int AUDIO_FREQ = 44100;
constexpr int SIMULATION_MULTIPLIER = 200;
constexpr float FRAMETIME = 16.f; /* ms */
constexpr size_t SIZE_LOG = 2.f / (0.001f * FRAMETIME);
constexpr float SIMULATION_FREQUENCY =
    SIMULATION_MULTIPLIER * 1000.f / FRAMETIME;
constexpr size_t AUDIO_SAMPLES =
    (SIMULATION_MULTIPLIER * AUDIO_FREQ / SIMULATION_FREQUENCY) * 1.05;
constexpr size_t CIRCULAR_BUFFER_SIZE = AUDIO_SAMPLES * 5;
constexpr size_t RESAMPLING_FACTOR = AUDIO_SAMPLES / SIMULATION_MULTIPLIER;

std::vector<float> sampledData(SIMULATION_MULTIPLIER);
constexpr size_t AUDIO_BUFFER_LENGTH = 10;
constexpr size_t GRAPHICS_BUFFER_LENGTH = 10;
int audio_cnt_gra = 0;
int audio_cnt_sim = 0;
std::array<float, AUDIO_SAMPLES> resampledBuffer;
std::vector<std::array<float, AUDIO_SAMPLES>> audioBuffer(AUDIO_BUFFER_LENGTH);

// Engine Controls
EngineControlsMgm engineControlsMgm;
// Logger Manager
LoggerMgm loggerMgm;

float cpuLoad = 0.f;

int counter_gra = 0;
int counter_sim = 0;
bool simulation_go = true;
std::binary_semaphore t1_semaphore{0};
std::vector<std::valarray<float>> piston_data(GRAPHICS_BUFFER_LENGTH);
size_t sim_idx = 0;
size_t gra_idx = 0;
int sim_overhead = 0;
int thread_multi = 1;

void audioCallback(void *userdata, Uint8 *stream, int len) {
  float *buffer = reinterpret_cast<float *>(stream);
  int samples = len / sizeof(float);

  for (int i = 0; i < samples; ++i) {
    buffer[i] = audioBuffer[audio_cnt_gra][i];
  }

  // Check simulation overhead
  sim_overhead = (audio_cnt_sim >= audio_cnt_gra)
                     ? audio_cnt_sim - audio_cnt_gra
                     : (audio_cnt_sim + AUDIO_BUFFER_LENGTH) - audio_cnt_gra;
  // Release simulation thread
  thread_multi = (DESIRED_SIM_OVERHEAD + 1) - sim_overhead;
  t1_semaphore.release();
  audio_cnt_gra = (audio_cnt_gra + 1) % AUDIO_BUFFER_LENGTH;
}

void simulation(EngineConfig const &cfg) {
  Piston piston = Piston(cfg);

  loggerMgm.addLogger("PistonP", CycleLogger([&piston]() {
                        return PAToATM(piston.gas.getP());
                      }));

  loggerMgm.addLogger(
      "IntakeFlow", CycleLogger([&piston]() { return piston.intakeFlow; }));

  loggerMgm.addLogger("PistonT", CycleLogger([&piston]() {
                        return KELVToCELS(piston.gas.getT());
                      }));

  loggerMgm.addLogger(
      "PistonOx", CycleLogger([&piston]() { return piston.gas.getOx(); }));

  loggerMgm.addLogger(
      "ExhaustFlow", CycleLogger([&piston]() { return piston.exhaustFlow; }));

  loggerMgm.addLogger(
      "Fuel", CycleLogger([&piston]() { return piston.gas.getFuel(); }));

  loggerMgm.addLogger("PistonV", CycleLogger([&piston]() {
                        return M3ToCC(piston.gas.getV());
                      }));

  loggerMgm.addLogger("IntakeP", CycleLogger([&piston]() {
                        return PAToATM(piston.intakeManifold.getP());
                      }));

  loggerMgm.addLogger("ExhaustP", CycleLogger([&piston]() {
                        return PAToATM(piston.exhaustPipe.getP());
                      }));
  loggerMgm.addLogger("IntakeOx", CycleLogger([&piston]() {
                        return piston.intakeManifold.getOx();
                      }));
  loggerMgm.addLogger("ExhaustOx", CycleLogger([&piston]() {
                        return piston.exhaustPipe.getOx();
                      }));

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

        sampledData[i] = ((PAToATM(piston.intakeManifold.getP()) - 1) +
                          (PAToATM(piston.exhaustPipe.getP()) - 1));

        loggerMgm.logAll();
        if (piston.cycleTrigger) {
          loggerMgm.resetAll();
          piston.cycleTrigger = false;
        }
      }

      // Queue the samples into the audio buffer
      audioBuffer[audio_cnt_sim] = resampleLinear<AUDIO_SAMPLES>(sampledData);
      audio_cnt_sim = (audio_cnt_sim + 1) % AUDIO_BUFFER_LENGTH;

      piston_data[sim_idx] = std::valarray<float>{piston.getCurrentAngle(),
                                                  piston.getThetaAngle(),
                                                  piston.intakeValve,
                                                  piston.exhaustValve};
      sim_idx = (sim_idx + 1) % GRAPHICS_BUFFER_LENGTH;

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
  desiredSpec.callback = audioCallback;

  // Open the audio device in non-callback mode (using queue).
  if (SDL_OpenAudio(&desiredSpec, nullptr) < 0) {
    std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

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

  // Start audio playback.
  SDL_PauseAudio(0);

  auto next_frame = std::chrono::high_resolution_clock::now() +
                    std::chrono::milliseconds(int(FRAMETIME));

  /* Game Loop */
  while (game.isGameRunning()) {
    // Get the current engine state
    engState = engineControlsMgm.getState();

    counter_gra++;

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::SliderFloat("Torque [Nm]", &engCtrls.externalTorque, 0.f, 20.f);
    ImGui::SliderFloat("Throttle", &engCtrls.throttle, 0.f, 1.f);
    ImGui::End();

    ImGui::Begin("ASD 1");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Thermodynamic Cycle",
                     loggerMgm.getLoggerData("PistonV"),
                     loggerMgm.getLoggerData("PistonP"),
                     loggerMgm.getLoggerSize("PistonP"));
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("ASD 2");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Intake flow",
                     loggerMgm.getLoggerData("IntakeFlow"),
                     loggerMgm.getLoggerSize("IntakeFlow"));
    ImPlot::PlotLine("Exhaust flow",
                     loggerMgm.getLoggerData("ExhaustFlow"),
                     loggerMgm.getLoggerSize("ExhaustFlow"));
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("ASD 3");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Chm Ox",
                     loggerMgm.getLoggerData("PistonOx"),
                     loggerMgm.getLoggerSize("PistonOx"));
    ImPlot::PlotLine("Int Ox",
                     loggerMgm.getLoggerData("IntakeOx"),
                     loggerMgm.getLoggerSize("IntakeOx"));
    ImPlot::PlotLine("Exh Ox",
                     loggerMgm.getLoggerData("ExhaustOx"),
                     loggerMgm.getLoggerSize("ExhaustOx"));
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("ASD 4");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Intake Pressure",
                     loggerMgm.getLoggerData("IntakeP"),
                     loggerMgm.getLoggerSize("IntakeP"));
    ImPlot::PlotLine("Exhaust Pressure",
                     loggerMgm.getLoggerData("ExhaustP"),
                     loggerMgm.getLoggerSize("ExhaustP"));
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Test8");
    ImGui::Text("Time:       %.1f s", 0.001f * gameLoopCnt * FRAMETIME);
    ImGui::Text("Framerate:  %.0f Hz", 1000.f / FRAMETIME);
    ImGui::Text("Simulation: %.0f Hz", SIMULATION_FREQUENCY);
    ImGui::Text("Engine Speed:  %.0f rpm", RADSToRPM(engState.engineSpeed));
    ImGui::Text("CPU Load:     %.0f / 100", 100 * cpuLoad);
    ImGui::Text("Graphics:       %d | %d | %d",
                counter_gra,
                counter_sim,
                counter_sim - counter_gra);
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
    gra_idx = (gra_idx + 1) % GRAPHICS_BUFFER_LENGTH;

    sim_overhead = (sim_idx >= gra_idx)
                       ? sim_idx - gra_idx
                       : (sim_idx + AUDIO_BUFFER_LENGTH) - gra_idx;

    // Wait for next frame, slow down the graphics thread if it's going too fast
    // or speed it up otherwise
    const int add_wait =
        ((sim_overhead < DESIRED_SIM_OVERHEAD) ? FRAMETIME / 4 : 0) -
        ((sim_overhead > DESIRED_SIM_OVERHEAD) ? FRAMETIME / 4 : 0);
    next_frame += std::chrono::milliseconds(int(FRAMETIME) + add_wait);
    std::this_thread::sleep_until(next_frame);
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

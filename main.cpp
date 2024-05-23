#include "FrameRVis.hpp"
#include "Game.hpp"
#include "Logger.hpp"
#include "Piston.hpp"
#include "PistonGraphics.hpp"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>

int SIMULATION_MULTIPLIER = 200;
float FRAMETIME = 20.f; /* ms */
float pistonX = 350.f;
float pistonY = 550.f;
float engineSpeed = 100.f;

float externalTorque = 0.f;

float average(std::vector<float> const &v) {
  if (v.empty()) {
    return 0;
  }

  auto const count = static_cast<float>(v.size());
  return std::reduce(v.begin(), v.end()) / count;
}

int main(int argc, char *argv[]) {
  printf("Program started\n");

  /* Game initialization */
  Game *game = new Game("Engine Simulator", SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED, 1000, 1000);
  FrameRVis *fVis = new FrameRVis();
  FrameRVis *load = new FrameRVis();
  CylinderGeometry *geom = new CylinderGeometry();
  Piston *piston = new Piston(*geom);
  CycleLogger *pistonPosLogger = new CycleLogger();
  CycleLogger *pressureLogger = new CycleLogger();
  CycleLogger *intakeLog = new CycleLogger();
  CycleLogger *exhaustLog = new CycleLogger();
  CycleLogger *torqueLog = new CycleLogger();
  CycleLogger *tempLog = new CycleLogger();
  CycleLogger *oxyLog = new CycleLogger();

  printf("Game initialized\n");

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSDL2_InitForSDLRenderer(game->window, game->renderer);
  ImGui_ImplSDLRenderer2_Init(game->renderer);

  printf("Start the game loop\n");

  /* Game Loop */
  while (game->isGameRunning()) {
    fVis->startClock();
    load->startClock();
    const int timeStart = SDL_GetTicks();
    PistonGraphics *pistonGraphics =
        new PistonGraphics(vector2_T{.x = pistonX, .y = pistonY}, piston, 2000);

    /* Simulation */
    for (size_t i = 0; i < SIMULATION_MULTIPLIER; ++i) {
      piston->updatePosition(FRAMETIME / (1000.f * SIMULATION_MULTIPLIER),
                             engineSpeed);

      piston->applyExtTorque(externalTorque);

      /* Log Data */
      pistonPosLogger->addSample(piston->getPistonPosition());
      pressureLogger->addSample(PAToATM(piston->gas->getP()));
      intakeLog->addSample(piston->intakeFlow);
      exhaustLog->addSample(piston->exhaustFlow);
      torqueLog->addSample(piston->getTorque());
      tempLog->addSample(KELVToCELS(piston->gas->getT()));
      oxyLog->addSample(piston->gas->getOx());

      if (piston->cycleTrigger) {
        pistonPosLogger->trig();
        pressureLogger->trig();
        intakeLog->trig();
        exhaustLog->trig();
        torqueLog->trig();
        tempLog->trig();
        oxyLog->trig();
        piston->cycleTrigger = false;
      }
    }

    const float avgTorque = average(torqueLog->getV());

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::Text("Framerate: %.1f fps", fVis->getFramerate());
    ImGui::Text("Speed: %.0f rpm", RADSToRPM(piston->omega));
    ImGui::Text("Load:      %.2f", 100.f * load->getLast() / FRAMETIME);
    ImGui::Text("Simul:     %.0f Hz",
                fVis->getFramerate() * SIMULATION_MULTIPLIER);

    ImGui::Text("Output torque: %.0f Nm", avgTorque);
    ImGui::Text("Output power:  %.0f W", avgTorque * piston->omega);

    ImGui::Checkbox("Activate dynamics", &piston->dynamicsIsActive);
    ImGui::Checkbox("Ignition", &piston->ignitionOn);
    ImGui::SliderFloat("Torque", &externalTorque, -20.f, 0.f);
    ImGui::SliderFloat("Throttle", &piston->throttle, 0.f, 1.f);
    ImGui::End();

    ImGui::Begin("Test2");
    ImGui::InputFloat("Engine speed", &engineSpeed, 0, 0, "%.0f", 0);
    ImGui::InputFloat("Combustion K", &piston->kexpl, 0, 0, "%.4f", 0);
    ImGui::InputFloat("Combustion Advance °", &piston->combustionAdvance, 0, 0,
                      "%.2f", 0);
    ImGui::InputFloat("Thermal K", &piston->thermalK, 0, 0, "%.4f", 0);
    ImGui::InputFloat("Intake K", &piston->intakeCoef, 0, 0, "%.4f", 0);
    ImGui::InputFloat("Exhaust K", &piston->exhaustCoef, 0, 0, "%.4f", 0);
    ImGui::InputFloat("Min Throttle", &piston->minThrottle, 0, 0, "%.4f", 0);
    ImGui::End();

    ImGui::Begin("Test3");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Torque", torqueLog->getData(),
                     std::min(torqueLog->getSize(), 15000));
    ImPlot::PlotLine("Oxy", oxyLog->getData(),
                     std::min(oxyLog->getSize(), 15000));
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Test4");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Pressure", pressureLogger->getData(),
                     std::min(pressureLogger->getSize(), 15000));
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Test5");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Temperature", tempLog->getData(),
                     std::min(tempLog->getSize(), 15000));
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Test6");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Intake", intakeLog->getData(),
                     std::min(intakeLog->getSize(), 15000));
    ImPlot::PlotLine("Exhaust", exhaustLog->getData(),
                     std::min(exhaustLog->getSize(), 15000));
    ImPlot::EndPlot();
    ImGui::End();

    /* Rendering */
    ImGui::Render();

    game->handleEvents();
    game->RenderClear();

    pistonGraphics->showPiston(game->renderer);

    load->endClock();

    /* Wait for next frame */
    int delay = FRAMETIME - (SDL_GetTicks() - timeStart);
    SDL_Delay((delay > 0) ? delay : 0);

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    game->RenderPresent();
    fVis->endClock();
  }

  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();

  game->Clean();
  return 0;
}

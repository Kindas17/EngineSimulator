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

constexpr int SIMULATION_MULTIPLIER = 400;
constexpr float FRAMETIME = 16.f; /* ms */
constexpr size_t SIZE_LOG = 2.f / (0.001f * FRAMETIME);

int main(int argc, char *argv[]) {

  bool start = false;
  size_t gameLoopCnt = 0;

  /* Game initialization */
  Game game = Game("Engine Simulator", SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED, 1000, 1000);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  ImGui_ImplSDL2_InitForSDLRenderer(game.window, game.renderer);
  ImGui_ImplSDLRenderer2_Init(game.renderer);

  CylinderGeometry geom = CylinderGeometry();
  Piston piston = Piston(geom);
  CycleLogger inflowLog = CycleLogger();
  CycleLogger outflowLog = CycleLogger();
  CycleLogger presLog = CycleLogger();
  CycleLogger voluLog = CycleLogger();
  CycleLogger nrLog = CycleLogger();
  CycleLogger tempLog = CycleLogger();
  CycleLogger oxyLog = CycleLogger();

  CycleLogger intakePresLog = CycleLogger();
  CycleLogger exhaustPresLog = CycleLogger();
  CycleLogger intakeTempLog = CycleLogger();
  CycleLogger exhaustTempLog = CycleLogger();

  // Initialize SDL_image (supports PNG, JPG, etc.)
  if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
    SDL_Log("SDL_image could not initialize! SDL_image Error: %s",
            IMG_GetError());
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    SDL_Quit();
    return -1;
  }

  /* Game Loop */
  while (game.isGameRunning()) {

    const int timeStart = SDL_GetTicks();

    PistonGraphics pistonGraphics = PistonGraphics(vector2_T{.x = 350.f, .y = 600.f}, &piston, 2000);

    if (start) {
      gameLoopCnt++;

      // Simulation

      for (size_t i = 0; i < SIMULATION_MULTIPLIER; ++i) {

        const float deltaT = getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME);
        const float t = 0.001f * (gameLoopCnt * FRAMETIME +
                                  i * FRAMETIME / SIMULATION_MULTIPLIER);

        piston.update(deltaT);

        intakePresLog.addSample(PAToATM(piston.intakeGas->getP()));
        exhaustPresLog.addSample(PAToATM(piston.exhaustGas->getP()));
        intakeTempLog.addSample(KELVToCELS(piston.intakeGas->getT()));
        exhaustTempLog.addSample(KELVToCELS(piston.exhaustGas->getT()));
        // presLog->addSample(PAToATM(piston.gas->getP()));
        // voluLog->addSample(M3ToCC(piston.gas->getV()));
        // nrLog->addSample(piston.gas->getnR());
        // tempLog->addSample(KELVToCELS(piston.gas->getT()));
        inflowLog.addSample(piston.gas->intakeFlow);
        outflowLog.addSample(piston.gas->exhaustFlow);
        // oxyLog->addSample(piston.gas->state[4]);

        if (piston.cycleTrigger) {
          intakePresLog.trig();
          exhaustPresLog.trig();
          intakeTempLog.trig();
          exhaustTempLog.trig();
          // presLog->trig();
          // voluLog->trig();
          // nrLog->trig();
          // tempLog->trig();
          inflowLog.trig();
          outflowLog.trig();
          // oxyLog->trig();
          piston.cycleTrigger = false;
        }

        // End Simulation
      }
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::SliderFloat("External Torque [Nm]", &piston.externalTorque, 0.f,
                       20.f);
    ImGui::SliderFloat("Intake Coef", &piston.intakeCoef, 0.00012f, 0.0012f,
                       "%.6f");
    ImGui::SliderFloat("Exhaust Coef", &piston.exhaustCoef, 0.00008f, 0.0008f,
                       "%.6f");
    ImGui::SliderFloat("Throttle", &piston.throttle, 0.f, 1.f);
    ImGui::InputFloat("Combustion speed", &piston.combustionSpeed);
    ImGui::InputFloat("Combustion energy", &piston.combustionEnergy);
    ImGui::InputFloat("Thermal conductivity", &piston.kthermal);
    ImGui::InputFloat("Advance", &piston.combustionAdvance);
    ImGui::InputFloat("Intake Timing", &piston.intakeTiming);
    ImGui::InputFloat("Exhaust Timing", &piston.exhaustTiming);
    ImGui::InputFloat("Intake Shape", &piston.intakeShape);
    ImGui::InputFloat("Exhaust Shape", &piston.exhaustShape);
    ImGui::End();

    ImGui::Begin("Test4");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Intake Flow", inflowLog.getData(), inflowLog.getSize());
    ImPlot::PlotLine("Exhaust Flow", outflowLog.getData(),
                     outflowLog.getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Temperature");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Intake Temp", intakeTempLog.getData(),
                     intakeTempLog.getSize());
    ImPlot::PlotLine("Exhaust Temp", exhaustTempLog.getData(),
                     exhaustTempLog.getSize());
    ImPlot::EndPlot();
    ImGui::End();

    // ImGui::Begin("Test5");
    // ImPlot::SetNextAxesToFit();
    // ImPlot::BeginPlot("ASD");
    // ImPlot::PlotLine("Oxygenation", oxyLog->getData(), oxyLog->getSize());
    // ImPlot::EndPlot();
    // ImGui::End();

    // ImGui::Begin("Test6");
    // ImPlot::SetNextAxesToFit();
    // ImPlot::BeginPlot("ASD");
    // ImPlot::PlotLine("Thermodynamic Cycle", voluLog->getData(),
    //                  presLog->getData(), presLog->getSize());
    // ImPlot::EndPlot();
    // ImGui::End();

    // ImGui::Begin("Test7");
    // ImPlot::SetNextAxesToFit();
    // ImPlot::BeginPlot("ASD");
    // ImPlot::PlotLine("Temperature", tempLog->getData(), tempLog->getSize());
    // ImPlot::EndPlot();
    // ImGui::End();

    ImGui::Begin("Pressure");
    ImPlot::SetNextAxesToFit();
    ImPlot::BeginPlot("ASD");
    ImPlot::PlotLine("Intake Pressure", intakePresLog.getData(),
                     intakePresLog.getSize());
    ImPlot::PlotLine("Exhaust Pressure", exhaustPresLog.getData(),
                     exhaustPresLog.getSize());
    ImPlot::EndPlot();
    ImGui::End();

    ImGui::Begin("Test8");
    ImGui::Text("Time:       %.1f s", 0.001f * gameLoopCnt * FRAMETIME);
    ImGui::Text("Framerate:  %.0f Hz", 1000.f / FRAMETIME);
    ImGui::Text("Simulation: %.0f Hz",
                SIMULATION_MULTIPLIER * 1000.f / FRAMETIME);
    ImGui::Text("Engine Speed:  %.0f rpm", RADSToRPM(piston.getEngineSpeed()));
    ImGui::Checkbox("Start", &start);
    ImGui::Checkbox("Ignition", &piston.ignitionOn);
    ImGui::End();

    ImGui::Begin("Intake Gas");
    ImGui::Text("Pressure: %.2f Atm", PAToATM(piston.intakeGas->getP()));
    ImGui::Text("Temperat: %.1f °C", KELVToCELS(piston.intakeGas->getT()));
    ImGui::Text("Oxygenat: %.2f", piston.intakeGas->getOx());
    ImGui::End();

    ImGui::Begin("Chamber Gas");
    ImGui::Text("Pressure: %.2f Atm", PAToATM(piston.gas->getP()));
    ImGui::Text("Temperat: %.1f °C", KELVToCELS(piston.gas->getT()));
    ImGui::Text("Oxygenat: %.2f", piston.gas->getOx());
    ImGui::End();

    ImGui::Begin("Exhaust Gas");
    ImGui::Text("Pressure: %.2f Atm", PAToATM(piston.exhaustGas->getP()));
    ImGui::Text("Temperat: %.1f °C", KELVToCELS(piston.exhaustGas->getT()));
    ImGui::Text("Oxygenat: %.2f", piston.exhaustGas->getOx());
    ImGui::End();

    /* Rendering */
    ImGui::Render();

    game.handleEvents();
    game.RenderClear();

    pistonGraphics.showPiston(game.renderer);

    /* Wait for next frame */
    int delay = FRAMETIME - (SDL_GetTicks() - timeStart);
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

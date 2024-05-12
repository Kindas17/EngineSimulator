#include "FrameRVis.hpp"
#include "Game.hpp"
#include "Logger.hpp"
#include "Piston.hpp"
#include "PistonGraphics.hpp"
#include <iomanip>
#include <iostream>

int SIMULATION_MULTIPLIER = 200;
float FRAMETIME = 20.f; /* ms */
float pistonX = 350.f;
float pistonY = 550.f;
float engineSpeed = 0.f;

float externalTorque = 0.f;

int main() {
  printf("Program started\n");

  /* Game initialization */
  Game *game = new Game("Engine Simulator", 0, 0, 700, 700);
  FrameRVis *fVis = new FrameRVis();
  FrameRVis *load = new FrameRVis();
  CylinderGeometry *geom = new CylinderGeometry();
  Piston *piston = new Piston(*geom);
  CycleLogger *pistonPosLogger = new CycleLogger();
  CycleLogger *pressureLogger = new CycleLogger();
  CycleLogger *intakeLog = new CycleLogger();
  CycleLogger *exhaustLog = new CycleLogger();

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
    load->startClock();
    const int timeStart = SDL_GetTicks();
    PistonGraphics *pistonGraphics = new PistonGraphics(
        (vector2_T){.x = pistonX, .y = pistonY}, piston, 2000);

    /* Simulation */
    for (size_t i = 0; i < SIMULATION_MULTIPLIER; ++i) {
      piston->updatePosition(FRAMETIME / (1000.f * SIMULATION_MULTIPLIER),
                             engineSpeed);
      
      piston->applyExtTorque(externalTorque);

      /* Log Data */
      pistonPosLogger->addSample(piston->getPistonPosition());
      pressureLogger->addSample(piston->gas->getP());
      intakeLog->addSample(piston->intakeFlow);
      exhaustLog->addSample(piston->exhaustFlow);

      if (piston->cycleTrigger) {
        pistonPosLogger->trig();
        pressureLogger->trig();
        intakeLog->trig();
        exhaustLog->trig();
        piston->cycleTrigger = false;
      }
    }

    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame(game->window);
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::Text("Framerate: %.1f fps", fVis->getFramerate());
    ImGui::Text("Speed: %.0f rpm", RADSToRPM(piston->omega));
    ImGui::Text("Load:      %.2f", 100.f * load->getLast() / FRAMETIME);
    ImGui::Text("Simul:     %.0f Hz",
                fVis->getFramerate() * SIMULATION_MULTIPLIER);

    // ImGui::InputFloat("Frametime [ms]", &FRAMETIME, 0, 0, "%.0f", 0);
    // ImGui::InputInt("Multiplier: ", &SIMULATION_MULTIPLIER);
    // ImGui::InputFloat("Engine speed", &engineSpeed, 0, 0, "%.0f", 0);
    ImGui::PlotLines("Pressure", pressureLogger->getData(),
                     pressureLogger->getSize());
    ImGui::PlotLines("Intake", intakeLog->getData(), intakeLog->getSize());
    ImGui::PlotLines("Exhaust", exhaustLog->getData(), exhaustLog->getSize());
    ImGui::Text("Pressure:    %.0f Pa (%.2f atm)", piston->gas->getP(),
                PAToATM(piston->gas->getP()));
    ImGui::Text("Volume:      %.2f cc", M3_TO_CC(piston->gas->getV()));
    ImGui::Text("Temperature: %.2f K (%.0f °C)", piston->gas->getT(),
                KELVToCELS(piston->gas->getT()));

    ImGui::SliderFloat("Torque", &externalTorque, 0.f, 50.f);
    ImGui::Checkbox("Activate dynamics", &piston->dynamicsIsActive);
    ImGui::End();

    ImGui::Begin("Test2");
    ImGui::InputFloat("Engine speed", &engineSpeed, 0, 0, "%.0f", 0);
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
  ImGui::DestroyContext();

  game->Clean();
  return 0;
}

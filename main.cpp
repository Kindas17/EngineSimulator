#include <iostream>
#include <iomanip>
#include "Game.hpp"
#include "FrameRVis.hpp"

float FRAMETIME = 40.f; /* ms */

int main()
{
    printf("Program started\n");

    /* Game initialization */
    Game* game = new Game("Engine Simulator", 0, 0, 700, 700);
    FrameRVis* fVis = new FrameRVis();

    printf("Game initialized\n");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL2_InitForSDLRenderer(game->window, game->renderer);
    ImGui_ImplSDLRenderer2_Init(game->renderer);

    printf("Start the game loop\n");

    /* Game Loop */
    while (game->isGameRunning())
    {
        fVis->startClock();
        const int timeStart = SDL_GetTicks();

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame(game->window);
        ImGui::NewFrame();

        ImGui::Begin("Test");
        ImGui::Text("Framerate: %.2f fps", fVis->getFramerate());
        ImGui::InputFloat( "Frametime [ms]", &FRAMETIME, 0, 0, "%.0f", 0 );
        ImGui::PlotLines("Framerate [fps]", fVis->getData(), fVis->getSize());
        if (ImGui::Button("Quit")) {
            game->QuitGame();
        };
        ImGui::End();
        ImGui::Render();

        /* Rendering */
        game->handleEvents();
        game->RenderClear();
        
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


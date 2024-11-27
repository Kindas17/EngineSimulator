#include "PistonGraphics.hpp"

// PistonGraphics::PistonGraphics(std::valarray<float> pos,
//                                Piston *p,
//                                int rFactor) {
//   this->crankCenter = pos;
//   this->piston = p;
//   this->rescaleFactor = rFactor;

//   /* Cylinder walls position */
//   this->cilinderRectPos = pos;
//   this->cilinderRectPos[1] -= rescaleFactor * p->cfg.rod.length +
//                               rescaleFactor * p->cfg.cylinder.stroke / 2.f;

//   // Load an image into a surface
//   pistonSurface = IMG_Load("assets/piston.png");
//   if (!pistonSurface) {
//     SDL_Log("Unable to load image! SDL_image Error: %s", IMG_GetError());
//   }
//   // Load an image into a surface
//   rodSurface = IMG_Load("assets/rod.png");
//   if (!rodSurface) {
//     SDL_Log("Unable to load image! SDL_image Error: %s", IMG_GetError());
//   }
// }

PistonGraphics::PistonGraphics(std::valarray<float> pos,
                               std::valarray<float> pistonData,
                               EngineConfig const &engineCfg,
                               int rFactor)
    : crankCenter{pos},
      rescaleFactor{rFactor},
      cilinderRectPos{pos},
      cfg{engineCfg},
      piston_data{pistonData} {
  /* Cylinder walls position */
  cilinderRectPos[1] -= rescaleFactor * cfg.rod.length +
                        rescaleFactor * cfg.cylinder.stroke / 2.f;

  // Load an image into a surface
  pistonSurface = IMG_Load("assets/piston.png");
  if (!pistonSurface) {
    SDL_Log("Unable to load image! SDL_image Error: %s", IMG_GetError());
  }
  // Load an image into a surface
  rodSurface = IMG_Load("assets/rod.png");
  if (!rodSurface) {
    SDL_Log("Unable to load image! SDL_image Error: %s", IMG_GetError());
  }
}

PistonGraphics::~PistonGraphics() {
  if (pistonSurface) {
    SDL_FreeSurface(pistonSurface);
    pistonSurface = nullptr;
  }
  if (rodSurface) {
    SDL_FreeSurface(rodSurface);
    rodSurface = nullptr;
  }
}

float PistonGraphics::getPistonPosition() {
  const float a = rodFoot[1];
  const float b = (rescaleFactor * cfg.cylinder.stroke / 2.f) *
                  (rescaleFactor * cfg.cylinder.stroke / 2.f) *
                  cos(piston_data[0]) * cos(piston_data[0]);
  const float c =
      b / (rescaleFactor * cfg.rod.length * rescaleFactor * cfg.rod.length);
  const float d = rescaleFactor * cfg.rod.length * sqrtf(1.f - c);
  return a - d;
}

void PistonGraphics::showPiston(SDL_Renderer *renderer) {
  /* Update engine geometry */
  rodFoot = {crankCenter[0] + (rescaleFactor * cfg.cylinder.stroke / 2.f) *
                                  cos(piston_data[0]),
             crankCenter[1] - (rescaleFactor * cfg.cylinder.stroke / 2.f) *
                                  sin(piston_data[0])};

  // Distance between piston head and connecting rod head
  const float pistonOverHead = 40.f;
  pistonPos = std::valarray<float>{crankCenter[0], getPistonPosition()};

  /* Combustion */
  // if (piston->ignitionOn) {
  SDL_Rect combustion;
  combustion.x = cilinderRectPos[0] - rescaleFactor * cfg.cylinder.bore / 2.f;
  combustion.y = cilinderRectPos[1] - cfg.cylinder.add_stroke * rescaleFactor +
                 2.f - pistonOverHead;
  combustion.w = rescaleFactor * cfg.cylinder.bore;
  combustion.h = -combustion.y + pistonPos[1];
  SDL_SetRenderDrawColor(renderer, 64, 32, 0, 0);
  // if (piston->getHeadAngle() >=
  //     std::numbers::pi - DEGToRAD(piston->combustionAdvance)) {
  //   SDL_RenderFillRect(renderer, &combustion);
  // }
  // }

  /* Draw the crankshaft */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
  SDL_RenderDrawLine(
      renderer, crankCenter[0], crankCenter[1], rodFoot[0], rodFoot[1]);
  SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0);

  /* Draw the cylinder */
  SDL_Rect cylinder;
  SDL_Rect addStroke;
  cylinder.x = cilinderRectPos[0] - rescaleFactor * cfg.cylinder.bore / 2.f;
  cylinder.y = cilinderRectPos[1] - pistonOverHead;
  cylinder.h = rescaleFactor * cfg.cylinder.stroke + pistonOverHead;
  cylinder.w = rescaleFactor * cfg.cylinder.bore;
  addStroke.x = cylinder.x;
  addStroke.y = cylinder.y - cfg.cylinder.add_stroke * rescaleFactor + 2.f;
  addStroke.h = cfg.cylinder.add_stroke * rescaleFactor;
  addStroke.w = cylinder.w;
  SDL_RenderDrawRect(renderer, &cylinder);
  SDL_RenderDrawRect(renderer, &addStroke);

  // /* Draw the rod and the piston */
  // SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);

  // // Rod
  // SDL_RenderDrawLine(
  //     renderer, rodFoot[0], rodFoot[1], pistonPos[0], pistonPos[1]);

  // Piston
  SDL_RenderDrawLine(renderer,
                     pistonPos[0] - rescaleFactor * cfg.cylinder.bore / 2.f,
                     pistonPos[1] - pistonOverHead,
                     pistonPos[0] + rescaleFactor * cfg.cylinder.bore / 2.f,
                     pistonPos[1] - pistonOverHead);

  // Convert surface to texture
  SDL_Texture *pistonTexture =
      SDL_CreateTextureFromSurface(renderer, pistonSurface);
  if (!pistonTexture) {
    SDL_Log(
        "Unable to create texture from surface! SDL_Error: %s", SDL_GetError());
  }
  // Convert surface to texture
  SDL_Texture *rodTexture = SDL_CreateTextureFromSurface(renderer, rodSurface);
  if (!rodTexture) {
    SDL_Log(
        "Unable to create texture from surface! SDL_Error: %s", SDL_GetError());
  }

  // Piston Texture
  SDL_Rect destRect;
  destRect.x = pistonPos[0] - rescaleFactor * cfg.cylinder.bore / 2.f;
  destRect.y = pistonPos[1] - pistonOverHead;
  destRect.w = rescaleFactor * cfg.cylinder.bore;
  destRect.h = rescaleFactor * cfg.cylinder.bore * 0.7f;
  SDL_RenderCopy(renderer, pistonTexture, nullptr, &destRect);

  // Rod Texture
  const float rodScaleFactor = 1.6f;
  destRect.x = rodFoot[0] - 25 * rodScaleFactor;
  destRect.y = rodFoot[1] - 60 * rodScaleFactor;
  destRect.w = 50 * rodScaleFactor;
  destRect.h = 80 * rodScaleFactor;
  SDL_Point center = {int(25 * rodScaleFactor), int(60 * rodScaleFactor)};
  SDL_RenderCopyEx(renderer,
                   rodTexture,
                   nullptr,
                   &destRect,
                   RADToDEG(-piston_data[1]),
                   &center,
                   SDL_FLIP_NONE);

  SDL_DestroyTexture(pistonTexture);
  SDL_DestroyTexture(rodTexture);

  // /* Draw Intake Valve */
  // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
  // const int intakeValveH = piston->intakeValve * 10.f;
  // SDL_RenderDrawLine(renderer,
  //                    addStroke.x + rescaleFactor * cfg.cylinder.bore / 4.f,
  //                    addStroke.y + intakeValveH,
  //                    addStroke.x + rescaleFactor * cfg.cylinder.bore / 4.f,
  //                    addStroke.y - 50.f + intakeValveH);
  // SDL_RenderDrawLine(
  //     renderer,
  //     addStroke.x + rescaleFactor * cfg.cylinder.bore / 4.f - 10.f,
  //     addStroke.y + intakeValveH,
  //     addStroke.x + rescaleFactor * cfg.cylinder.bore / 4.f + 10.f,
  //     addStroke.y + intakeValveH);

  // const int exhaustValveH = piston->exhaustValve * 10.f;
  // SDL_RenderDrawLine(
  //     renderer,
  //     addStroke.x + 3.f * rescaleFactor * cfg.cylinder.bore / 4.f,
  //     addStroke.y + exhaustValveH,
  //     addStroke.x + 3.f * rescaleFactor * cfg.cylinder.bore / 4.f,
  //     addStroke.y - 50.f + exhaustValveH);
  // SDL_RenderDrawLine(
  //     renderer,
  //     addStroke.x + 3.f * rescaleFactor * cfg.cylinder.bore / 4.f - 10.f,
  //     addStroke.y + exhaustValveH,
  //     addStroke.x + 3.f * rescaleFactor * cfg.cylinder.bore / 4.f + 10.f,
  //     addStroke.y + exhaustValveH);
}

#include "PistonGraphics.hpp"

PistonGraphics::PistonGraphics(vector2_T pos, Piston *p, int rFactor) {
  this->crankCenter = pos;
  this->piston = p;
  this->rescaleFactor = rFactor;

  /* Cylinder walls position */
  this->cilinderRectPos = pos;
  this->cilinderRectPos.y -= rescaleFactor * p->geometry.rod +
                             rescaleFactor * p->geometry.stroke / 2.f;

  // Load an image into a surface
  pistonSurface = IMG_Load("piston.png");
  if (!pistonSurface) {
    SDL_Log("Unable to load image! SDL_image Error: %s", IMG_GetError());
  }
  // Load an image into a surface
  rodSurface = IMG_Load("rod.png");
  if (!rodSurface) {
    SDL_Log("Unable to load image! SDL_image Error: %s", IMG_GetError());
  }
}

float PistonGraphics::getPistonPosition() {
  const float a = rodFoot.y;
  const float b = (rescaleFactor * piston->geometry.stroke / 2.f) *
                  (rescaleFactor * piston->geometry.stroke / 2.f) *
                  cos(piston->getCurrentAngle()) *
                  cos(piston->getCurrentAngle());
  const float c = b / (rescaleFactor * piston->geometry.rod * rescaleFactor *
                       piston->geometry.rod);
  const float d = rescaleFactor * piston->geometry.rod * sqrtf(1.f - c);
  return a - d;
}

void PistonGraphics::showPiston(SDL_Renderer *renderer) {
  /* Update engine geometry */
  rodFoot = {
      .x = crankCenter.x + (rescaleFactor * piston->geometry.stroke / 2.f) *
                               cos(piston->getCurrentAngle()),
      .y = crankCenter.y - (rescaleFactor * piston->geometry.stroke / 2.f) *
                               sin(piston->getCurrentAngle())};

  pistonPos = {.x = crankCenter.x, .y = getPistonPosition()};

  /* Combustion */
  if (piston->ignitionOn) {
    SDL_Rect combustion;
    combustion.x =
        cilinderRectPos.x - rescaleFactor * piston->geometry.bore / 2.f;
    combustion.y =
        cilinderRectPos.y - piston->geometry.addStroke * rescaleFactor + 2.f;
    combustion.w = rescaleFactor * piston->geometry.bore;
    combustion.h = -combustion.y + pistonPos.y;
    SDL_SetRenderDrawColor(renderer, 64, 32, 0, 0);
    if (piston->getHeadAngle() >=
        std::numbers::pi - piston->combustionAdvance) {
      SDL_RenderFillRect(renderer, &combustion);
    }
  }

  /* Draw the crankshaft */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
  SDL_RenderDrawLine(renderer, crankCenter.x, crankCenter.y, rodFoot.x,
                     rodFoot.y);
  SDL_SetRenderDrawColor(renderer, 64, 64, 64, 0);
  /* Draw the cylinder */
  SDL_Rect cylinder;
  SDL_Rect addStroke;
  cylinder.x = cilinderRectPos.x - rescaleFactor * piston->geometry.bore / 2.f;
  cylinder.y = cilinderRectPos.y;
  cylinder.h = rescaleFactor * piston->geometry.stroke;
  cylinder.w = rescaleFactor * piston->geometry.bore;
  addStroke.x = cylinder.x;
  addStroke.y = cylinder.y - piston->geometry.addStroke * rescaleFactor + 2.f;
  addStroke.h = piston->geometry.addStroke * rescaleFactor;
  addStroke.w = cylinder.w;
  SDL_RenderDrawRect(renderer, &cylinder);
  SDL_RenderDrawRect(renderer, &addStroke);
  /* Draw the rod and the piston */
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
  SDL_RenderDrawLine(renderer, rodFoot.x, rodFoot.y, pistonPos.x, pistonPos.y);
  // SDL_RenderDrawLine(
  //     renderer, pistonPos.x - rescaleFactor * piston->geometry.bore / 2.f,
  //     pistonPos.y, pistonPos.x + rescaleFactor * piston->geometry.bore / 2.f,
  //     pistonPos.y);

  // Convert surface to texture
  SDL_Texture *pistonTexture =
      SDL_CreateTextureFromSurface(renderer, pistonSurface);
  if (!pistonTexture) {
    SDL_Log("Unable to create texture from surface! SDL_Error: %s",
            SDL_GetError());
  }
  // Convert surface to texture
  SDL_Texture *rodTexture = SDL_CreateTextureFromSurface(renderer, rodSurface);
  if (!rodTexture) {
    SDL_Log("Unable to create texture from surface! SDL_Error: %s",
            SDL_GetError());
  }

  // Copy the texture to the rendering target (the window)
  SDL_Rect destRect;
  destRect.x =
      pistonPos.x - rescaleFactor * piston->geometry.bore / 2.f; // x position
  destRect.y = pistonPos.y;                                      // y position
  destRect.w =
      rescaleFactor * piston->geometry.bore; // width of the texture on screen
  destRect.h = rescaleFactor * piston->geometry.bore *
               0.7f; // height of the texture on screen
  SDL_RenderCopy(renderer, pistonTexture, nullptr, &destRect);

  // Copy the texture to the rendering target (the window)
  destRect.x = rodFoot.x - 25; // x
  destRect.y = rodFoot.y - 60; // y
  destRect.w = 50;             // width of the texture on
  destRect.h = 80;             // height of the texture on screen
  // SDL_RenderCopy(renderer, rodTexture, nullptr, &destRect);
  SDL_Point center = {(int)destRect.x, (int)destRect.y};
  SDL_RenderCopyEx(renderer, rodTexture, nullptr, &destRect,
                   RADToDEG(-piston->getThetaAngle()), nullptr, SDL_FLIP_NONE);

  /* Draw Intake Valve */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
  const int intakeValveH = piston->intakeValve * 10.f;
  SDL_RenderDrawLine(renderer,
                     addStroke.x + rescaleFactor * piston->geometry.bore / 4.f,
                     addStroke.y + intakeValveH,
                     addStroke.x + rescaleFactor * piston->geometry.bore / 4.f,
                     addStroke.y - 50.f + intakeValveH);
  SDL_RenderDrawLine(
      renderer,
      addStroke.x + rescaleFactor * piston->geometry.bore / 4.f - 10.f,
      addStroke.y + intakeValveH,
      addStroke.x + rescaleFactor * piston->geometry.bore / 4.f + 10.f,
      addStroke.y + intakeValveH);

  const int exhaustValveH = piston->exhaustValve * 10.f;
  SDL_RenderDrawLine(
      renderer, addStroke.x + 3.f * rescaleFactor * piston->geometry.bore / 4.f,
      addStroke.y + exhaustValveH,
      addStroke.x + 3.f * rescaleFactor * piston->geometry.bore / 4.f,
      addStroke.y - 50.f + exhaustValveH);
  SDL_RenderDrawLine(
      renderer,
      addStroke.x + 3.f * rescaleFactor * piston->geometry.bore / 4.f - 10.f,
      addStroke.y + exhaustValveH,
      addStroke.x + 3.f * rescaleFactor * piston->geometry.bore / 4.f + 10.f,
      addStroke.y + exhaustValveH);
}

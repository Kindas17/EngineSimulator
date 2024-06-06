#include "PistonGraphics.hpp"

PistonGraphics::PistonGraphics(vector2_T pos, Piston *piston,
                               int rescaleFactor) {
  this->crankCenter = pos;
  this->piston = piston;
  this->rescaleFactor = rescaleFactor;

  /* Cylinder walls position */
  this->cilinderRectPos = pos;
  this->cilinderRectPos.y -= rescaleFactor * piston->geometry.rod +
                             rescaleFactor * piston->geometry.stroke / 2;
}

float PistonGraphics::getPistonPosition() {
  const float a = rodFoot.y;
  const float b = (rescaleFactor * piston->geometry.stroke / 2) *
                  (rescaleFactor * piston->geometry.stroke / 2) *
                  cosf(DEGToRAD(piston->currentAngle)) *
                  cosf(DEGToRAD(piston->currentAngle));
  const float c = b / (rescaleFactor * piston->geometry.rod * rescaleFactor *
                       piston->geometry.rod);
  const float d = rescaleFactor * piston->geometry.rod * sqrtf(1 - c);
  return a - d;
}

void PistonGraphics::showPiston(SDL_Renderer *renderer) {
  /* Update engine geometry */
  rodFoot = {
      .x = crankCenter.x + (rescaleFactor * piston->geometry.stroke / 2) *
                               cosf(DEGToRAD(piston->currentAngle)),
      .y = crankCenter.y - (rescaleFactor * piston->geometry.stroke / 2) *
                               sinf(DEGToRAD(piston->currentAngle))};

  pistonPos = {.x = crankCenter.x, .y = getPistonPosition()};

  /* Combustion */
  if (piston->ignitionOn) {
    SDL_Rect combustion;
    combustion.x =
        cilinderRectPos.x - rescaleFactor * piston->geometry.bore / 2;
    combustion.y =
        cilinderRectPos.y - piston->geometry.addStroke * rescaleFactor + 2;
    combustion.w = rescaleFactor * piston->geometry.bore;
    combustion.h = -combustion.y + pistonPos.y;
    SDL_SetRenderDrawColor(renderer, 64, 32, 0, 0);
    if (piston->state[0] >= 180 - piston->combustionAdvance) {
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
  cylinder.x = cilinderRectPos.x - rescaleFactor * piston->geometry.bore / 2;
  cylinder.y = cilinderRectPos.y;
  cylinder.h = rescaleFactor * piston->geometry.stroke;
  cylinder.w = rescaleFactor * piston->geometry.bore;
  addStroke.x = cylinder.x;
  addStroke.y = cylinder.y - piston->geometry.addStroke * rescaleFactor + 2;
  addStroke.h = piston->geometry.addStroke * rescaleFactor;
  addStroke.w = cylinder.w;
  SDL_RenderDrawRect(renderer, &cylinder);
  SDL_RenderDrawRect(renderer, &addStroke);
  /* Draw the rod and the piston */
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0);
  SDL_RenderDrawLine(renderer, rodFoot.x, rodFoot.y, pistonPos.x, pistonPos.y);
  SDL_RenderDrawLine(
      renderer, pistonPos.x - rescaleFactor * piston->geometry.bore / 2,
      pistonPos.y, pistonPos.x + rescaleFactor * piston->geometry.bore / 2,
      pistonPos.y);

  /* Draw Intake Valve */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
  const int intakeValveH = piston->intakeValve * 10;
  SDL_RenderDrawLine(renderer,
                     addStroke.x + rescaleFactor * piston->geometry.bore / 4,
                     addStroke.y + intakeValveH,
                     addStroke.x + rescaleFactor * piston->geometry.bore / 4,
                     addStroke.y - 50 + intakeValveH);
  SDL_RenderDrawLine(
      renderer, addStroke.x + rescaleFactor * piston->geometry.bore / 4 - 10,
      addStroke.y + intakeValveH,
      addStroke.x + rescaleFactor * piston->geometry.bore / 4 + 10,
      addStroke.y + intakeValveH);

  const int exhaustValveH = piston->exhaustValve * 10;
  SDL_RenderDrawLine(
      renderer, addStroke.x + 3 * rescaleFactor * piston->geometry.bore / 4,
      addStroke.y + exhaustValveH,
      addStroke.x + 3 * rescaleFactor * piston->geometry.bore / 4,
      addStroke.y - 50 + exhaustValveH);
  SDL_RenderDrawLine(
      renderer,
      addStroke.x + 3 * rescaleFactor * piston->geometry.bore / 4 - 10,
      addStroke.y + exhaustValveH,
      addStroke.x + 3 * rescaleFactor * piston->geometry.bore / 4 + 10,
      addStroke.y + exhaustValveH);
}
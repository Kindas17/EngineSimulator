#ifndef PISTON_GRAPHICS_HPP
#define PISTON_GRAPHICS_HPP
#include "Linalg.hpp"
#include "Piston.hpp"
#include <SDL2/SDL.h>
#include <stdio.h>

class PistonGraphics {
public:
  PistonGraphics(vector2_T pos, Piston *piston, int rescaleFactor);
  void showPiston(SDL_Renderer *renderer);
  float getPistonPosition();

  Piston *piston;

  /* GGeometry */
  vector2_T crankCenter;
  vector2_T rodFoot;
  vector2_T pistonPos;
  vector2_T cilinderRectPos;

  int rescaleFactor;
};

#endif

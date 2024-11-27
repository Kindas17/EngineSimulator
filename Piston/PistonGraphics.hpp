#ifndef PISTON_GRAPHICS_HPP
#define PISTON_GRAPHICS_HPP
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#include <valarray>

#include "EngineConfig.hpp"
#include "Piston.hpp"

class PistonGraphics {
 public:
  // PistonGraphics(std::valarray<float> pos, Piston *p, int rescaleFactor);
  PistonGraphics(std::valarray<float> pos,
                 std::valarray<float> pistonData,
                 EngineConfig const &engineCfg,
                 int rFactor);
  void showPiston(SDL_Renderer *renderer);
  float getPistonPosition();

  EngineConfig const &cfg;
  // [getCurrentAngle, ]
  std::valarray<float> piston_data;

  /* GGeometry */
  std::valarray<float> crankCenter;
  std::valarray<float> rodFoot;
  std::valarray<float> pistonPos;
  std::valarray<float> cilinderRectPos;

  SDL_Surface *pistonSurface;
  SDL_Surface *rodSurface;

  int rescaleFactor;
};

#endif

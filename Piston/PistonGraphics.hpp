#ifndef PISTON_GRAPHICS_HPP
#define PISTON_GRAPHICS_HPP
#include <stdio.h>
#include <SDL2/SDL.h>
#include "Piston.hpp"
#include "Linalg.hpp"


class PistonGraphics
{
public:
    PistonGraphics(vector2_T pos, Piston* piston, int rescaleFactor);
    void showPiston(SDL_Renderer* renderer);
    float getPistonPosition();

    Piston* piston;

    /* GGeometry */
    vector2_T crankCenter;
    vector2_T rodFoot;
    vector2_T pistonPos;
    vector2_T cilinderRectPos;

    int rescaleFactor;
};



#endif

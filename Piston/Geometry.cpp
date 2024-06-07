#include "Geometry.hpp"
#include <cmath>

constexpr float CRANKSHAFT_L = 25.0f;            /* [mm] */
constexpr float BIELLA_L = 55.0f;                /* [mm] */
constexpr float ADD_STROKE = CRANKSHAFT_L / 3.f; /* No Unit */

CylinderGeometry::CylinderGeometry()
    : stroke(MMToM(CRANKSHAFT_L * 2.f)), addStroke(MMToM(ADD_STROKE)),
      rod(MMToM(BIELLA_L)), bore(MMToM(50.6f)),
      momentOfInertia(25.f * pow(stroke * 0.5f, 2)) {}

#include "IdealGas.hpp"
#include <cmath>

IdealGas::IdealGas(float p, float v, float t) {

  pressure = p;
  volume = v;
  temperature = t;
  nR = pressure * volume / temperature;
}

void IdealGas::AdiabaticCompress(float vprime, float dt) {

  /* Compute the pressure variation */
  const float p0 = pressure;
  const float v0 = volume;
  const float v = v0 + vprime * dt;
  const float p = p0 * std::pow((v / v0), -1.4f);

  /* Update the status */
  pressure = p;
  volume = v;
  temperature = pressure * volume / nR;
}

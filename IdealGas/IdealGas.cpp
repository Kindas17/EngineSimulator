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

void IdealGas::SimpleFlow(float kFlow, float ext_pressure, float dt) {

  /* p(t) = c1 * exp(-a k t) + b */

  /* Compute the pressure variation */
  const float a = temperature / volume;
  const float b = ext_pressure;
  const float p0 = pressure;
  const float c1 = p0 - b;
  const float p = c1 * std::exp(-a * kFlow * dt) + b;

  const float deltaP = (b - p0);
  const float nrPrime = kFlow * deltaP;

  /* Update the status */
  pressure = p;
  nR += nrPrime * dt;
  temperature = pressure * volume / nR;
}

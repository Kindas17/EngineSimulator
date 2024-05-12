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

float IdealGas::SimpleFlow(float kFlow, float ext_pressure, float ext_temp,
                           float dt) {

  /* SIMPLIFIED VERSION - TEMPERATURE VARIATION */
  /* Compute the pressure variation */
  const float a = temperature / volume;
  const float b = ext_pressure;
  const float p0 = pressure;
  const float c1 = p0 - b;
  const float p = c1 * std::exp(-a * kFlow * dt) + b;

  const float deltaP = (b - p0);
  const float nrPrime = kFlow * deltaP;

  const float tout = ext_temp;
  const float t0 = temperature;
  const float nr0 = nR;

  /* Update the status */
  pressure = p;
  nR += nrPrime * dt;

#ifdef NO_TEMP_VARIATION
  temperature = pressure * volume / nR;
#else

  /* Weighted average of entering and internal fluid */
  const float adjT = pressure * volume / nR;
  const float t = (dt * nrPrime * tout + nr0 * adjT) / nR;

  temperature = t;

#endif

  return nrPrime * dt;
}

void IdealGas::HeatExchange(float kTherm, float ext_temp, float dt) {

  const float t0 = temperature;
  const float p0 = pressure;
  const float c = ext_temp;
  const float b = volume;
  const float r = nR;

  const float y1 = t0 - c;
  const float y2 = p0 - c * r / b;

  const float expo = std::exp(kTherm * dt / (alpha * r));
  const float t = y1 * expo + c;
  const float p = (1 - expo) * r * y1 / b + y2 + c * r / b;

  /* Update the status */
  temperature = t;
  pressure = p;
}

#include "IdealGas.hpp"
#include <cmath>

using namespace std::numbers;

IdealGas::IdealGas(float p, float v, float t) {

  state[0] = p;
  state[1] = v;
  state[3] = t;
  state[2] = state[0] * state[1] / state[3];
}

std::valarray<float> F(float t, std::valarray<float> &state, float VPrime) {

  const float alpha = IdealGas::alpha;
  return std::valarray<float>{
      -(1.f + 1.f / alpha) * state[0] * VPrime / state[1], // P'
      VPrime,                                              // V'
      0.f,                                                 // nR'
      -state[0] * VPrime / (alpha * state[2])              // T'
  };
}

// void IdealGas::AdiabaticCompress(float vprime, float dt) {

//   /* Compute the pressure variation */
//   const float p0 = pressure;
//   const float v0 = volume;
//   const float v = v0 + vprime * dt;
//   const float p = p0 * std::pow((v / v0), -1.4f);

//   /* Update the status */
//   pressure = p;
//   volume = v;
//   temperature = pressure * volume / nR;
// }

// float IdealGas::SimpleFlow(float kFlow, float ext_pressure, float ext_temp,
//                            float dt) {

//   /* SIMPLIFIED VERSION - TEMPERATURE VARIATION */
//   /* Compute the pressure variation */
//   const float a = temperature / volume;
//   const float b = ext_pressure;
//   const float p0 = pressure;
//   const float c1 = p0 - b;
//   const float p = c1 * std::exp(-a * kFlow * dt) + b;

//   const float deltaP = (b - p0);
//   const float nrPrime = kFlow * deltaP;

//   const float tout = ext_temp;
//   const float nr0 = nR;

//   /* Update the status */
//   pressure = p;
//   nR += nrPrime * dt;

//   /* Weighted average of entering and internal fluid */
//   const float adjT = pressure * volume / nR;
//   const float t = (dt * nrPrime * tout + nr0 * adjT) / nR;

//   temperature = t;

//   return nrPrime * dt;
// }

// void IdealGas::HeatExchange(float kTherm, float ext_temp, float dt) {

//   const float t0 = temperature;
//   const float p0 = pressure;
//   const float c = ext_temp;
//   const float b = volume;
//   const float r = nR;

//   const float y1 = t0 - c;
//   const float y2 = p0 - c * r / b;

//   const float expo = std::exp(kTherm * dt / (alpha * r));
//   const float t = y1 * expo + c;
//   const float p = (1 - expo) * r * y1 / b + y2 + c * r / b;

//   /* Update the status */
//   temperature = t;
//   pressure = p;
// }

// Gas::Gas(float p, float v, float t, float o) : IdealGas::IdealGas(p, v, t) {

//   ox = o;
// }

// float Gas::SimpleFlow(float kFlow, float ext_pressure, float ext_temp,
//                       float ext_ox, float dt) {

//   /* SIMPLIFIED VERSION - TEMPERATURE VARIATION */
//   /* Compute the pressure variation */
//   const float a = temperature / volume;
//   const float b = ext_pressure;
//   const float p0 = pressure;
//   const float c1 = p0 - b;
//   const float p = c1 * std::exp(-a * kFlow * dt) + b;

//   const float deltaP = (b - p0);
//   const float nrPrime = kFlow * deltaP;

//   const float tout = ext_temp;
//   const float nr0 = nR;

//   /* Update the status */
//   pressure = p;
//   nR += nrPrime * dt;

//   /* Weighted average of entering and internal fluid */
//   if (nrPrime > 0) {
//     const float ox0 = ox;
//     const float ox1 = (dt * nrPrime * ext_ox + nr0 * ox0) / nR;

//     ox = ox1;

//     const float adjT = pressure * volume / nR;
//     const float t = (dt * nrPrime * tout + nr0 * adjT) / nR;

//     temperature = t;
//   }

//   return nrPrime * dt;
// }

// void Gas::InjectHeat(float kx, float dt) {

//   const float t0 = temperature;
//   const float p0 = pressure;

//   const float qprime = 10000.0f * kx * nR * ox / dt;

//   const float t = t0 + dt * qprime / (alpha * nR);
//   const float p = p0 + dt * qprime / (alpha * volume);

//   temperature = t;
//   pressure = p;
//   ox *= (1.f - kx);
// }

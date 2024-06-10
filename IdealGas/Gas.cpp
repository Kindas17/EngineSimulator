#include "Gas.hpp"

#include <iostream>

std::valarray<float> F_Gas(float t, std::valarray<float> &st, float ang,
                           float omega, CylinderGeometry g, float nRPrime,
                           float QPrime, float oxPrime) {

  auto stPrime = F_IdealGas(t, st, ang, omega, g, nRPrime, QPrime);
  stPrime[4] = oxPrime;

  return stPrime;
}

Gas::Gas(float p, float v, float t, float o) : IdealGas(p, v, t) {

  state[4] = o;
  oxPrime = 0.f;
}

void Gas::updateState(float kthermal, float kFlow_int, float kFlow_exh,
                      float Pout_int, float Pout_exh, float Tout_int,
                      float Tout_exh, float ox_int, float ox_exh, float kcs,
                      float kce) {

  IdealGas::updateState(kthermal, kFlow_int, kFlow_exh, Pout_int, Pout_exh,
                        Tout_int, Tout_exh);

  const float k_oxy = 50.f;

  const float oxPrime_int =
      (intakeFlow > 0.f) ? k_oxy * intakeFlow * (ox_int - state[4]) : 0.f;
  const float oxPrime_exh =
      (exhaustFlow > 0.f) ? k_oxy * exhaustFlow * (ox_exh - state[4]) : 0.f;

  // Combustion
  const float oxPrime_combustion = -kcs * state[2] * state[4];
  QPrime += -kce * oxPrime_combustion;

  oxPrime = oxPrime_int + oxPrime_exh + oxPrime_combustion;
}

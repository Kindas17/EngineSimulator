#include "Gas.hpp"

#include <iostream>

std::valarray<float> F_Gas(float t,
                           std::valarray<float> &st,
                           std::valarray<float> stp) {
  auto stPrime = F_IdealGas(t, st, stp);
  stPrime[4] = stp[3];
  stPrime[5] = stp[4];

  return stPrime;
}

Gas::Gas(float p, float v, float t, float o) : IdealGas(p, v, t) {
  state[4] = o;
  state[5] = 0.f;
  oxPrime = 0.f;
  fuelPrime = 0.f;
  fuelInjected = 0.f;
}

std::valarray<float> Gas::combust(float kcs, float kce) {
  float oxPrime = -100 * kcs * state[2] * state[4];
  float fuelPrime = -kcs * state[5];
  float QPrime = -fuelPrime * kce;
  return std::valarray<float>{0.f, 0.f, QPrime, oxPrime, fuelPrime};
}

void Gas::updateState(float kthermal,
                      float kFlow_int,
                      float kFlow_exh,
                      float Pout_int,
                      float Pout_exh,
                      float Tout_int,
                      float Tout_exh,
                      float ox_int,
                      float ox_exh,
                      float kcs,
                      float kce) {
  IdealGas::updateState(
      kthermal, kFlow_int, kFlow_exh, Pout_int, Pout_exh, Tout_int, Tout_exh);

  // const float k_oxy = 50.f;

  // const float oxPrime_int =
  //     (intakeFlow > 0.f) ? k_oxy * intakeFlow * (ox_int - state[4]) : 0.f;
  // const float oxPrime_exh =
  //     (exhaustFlow > 0.f) ? k_oxy * exhaustFlow * (ox_exh - state[4]) : 0.f;

  // // Combustion
  // const float oxPrime_combustion = -kcs * state[2] * state[4];
  // // QPrime += -kce * oxPrime_combustion;

  // fuelPrime = -100.f * state[5];
  // QPrime += -fuelPrime * kce * 100000.f;

  // oxPrime = oxPrime_int + oxPrime_exh + oxPrime_combustion;
}

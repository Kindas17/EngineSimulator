#ifndef GAS_HPP
#define GAS_HPP

#include "IdealGas.hpp"

std::valarray<float> F_Gas(float t,
                           std::valarray<float> &st,
                           std::valarray<float> stp);

class Gas : public IdealGas {
 public:
  float oxPrime;
  float fuelPrime;
  float fuelInjected;

  Gas(float p, float v, float t, float o);

  void updateState(float kthermal,
                   float kFlow_int,
                   float kFlow_exh,
                   float Pout_int,
                   float Pout_exh,
                   float Tout_int,
                   float Tout_exh,
                   float ox_int,
                   float ox_exh,
                   float kcs,
                   float kce);

  float getOx() {
    return state[4];
  }

  float getFuel() {
    return state[5];
  }

  void setFuelAmnt(float afr) {
    const float airMass = M_air * state[2] / IDEAL_GAS_CONSTANT;
    state[5] = state[4] * airMass / afr;
    fuelInjected = state[5];
  }
};

#endif

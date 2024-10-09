#ifndef GAS_HPP
#define GAS_HPP

#include "IdealGas.hpp"

std::valarray<float> F_Gas(float t,
                           std::valarray<float> &st,
                           float ang,
                           float omega,
                           CylinderGeometry g,
                           float nRPrime,
                           float QPrime,
                           float oxPrime);

class Gas : public IdealGas {
 public:
  float oxPrime;

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
};

#endif

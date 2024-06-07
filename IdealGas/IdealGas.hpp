#ifndef IDEALGAS_HPP
#define IDEALGAS_HPP

#include "Geometry.hpp"
#include <valarray>

constexpr float DEFAULT_AMBIENT_PRESSURE = 101325.f;
constexpr float DEFAULT_AMBIENT_TEMPERATURE = 300.f;
constexpr float ZERO_CELSIUS_IN_KELVIN = 273.15f;
constexpr float PAToATM(float X) { return ((X) / DEFAULT_AMBIENT_PRESSURE); }
constexpr float KELVToCELS(float X) { return ((X)-ZERO_CELSIUS_IN_KELVIN); }

std::valarray<float> F(float t, std::valarray<float> &st, float ang,
                       float omega, CylinderGeometry g, float nRPrime,
                       float QPrime);

class IdealGas {

public:
  float getP() { return state[0]; }
  float getV() { return state[1]; }
  float getnR() { return state[2]; }
  float getT() { return state[3]; }

  IdealGas(float p, float v, float t);

  static constexpr float alpha = 5.f / 2.f;
  std::valarray<float> state = {0.f, 0.f, 0.f, 0.f};

  void updateState(float Vp, float kFlow_int, float kFlow_exh, float Pout_int,
                   float Pout_exh, float Tout_int, float Tout_exh);

  float QPrime;
  float nRPrime;
  float VPrime;
};

#endif

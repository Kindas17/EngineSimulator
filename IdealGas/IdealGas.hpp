#ifndef IDEALGAS_HPP
#define IDEALGAS_HPP

#include <valarray>

#include "Geometry.hpp"

constexpr float IDEALGAS_ALPHA = 5.f / 2.f;
constexpr float IDEAL_GAS_CONSTANT = 8.314f;
// Approx molar mass of air in kg/mol
constexpr float M_air = 0.029f;
constexpr float DEFAULT_AMBIENT_PRESSURE = 101325.f;
constexpr float DEFAULT_AMBIENT_TEMPERATURE = 300.f;
constexpr float ZERO_CELSIUS_IN_KELVIN = 273.15f;
constexpr float PAToATM(float X) {
  return ((X) / DEFAULT_AMBIENT_PRESSURE);
}
constexpr float KELVToCELS(float X) {
  return ((X)-ZERO_CELSIUS_IN_KELVIN);
}

float gasFlowFunction(float Pup, float Pdown, float Tup, float Tdown);

std::valarray<float> F_IdealGas(float t,
                                std::valarray<float> &st,
                                std::valarray<float> stp);

class IdealGas {
 public:
  float getP() {
    return state[0];
  }
  float getV() {
    return state[1];
  }
  float getnR() {
    return state[2];
  }
  float getT() {
    return state[3];
  }

  IdealGas() {}
  IdealGas(float p, float v, float t);

  std::valarray<float> exchangeHeat(float kTherm, float extTemp);

  static constexpr float alpha = 5.f / 2.f;

  // The last value is just to solve a stupid problem...
  // Derived class Gas needs 6 elements here.
  std::valarray<float> state = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};

  void updateState(float kthermal,
                   float kFlow_int,
                   float kFlow_exh,
                   float Pout_int,
                   float Pout_exh,
                   float Tout_int,
                   float Tout_exh);

  float QPrime;
  float nRPrime;
  float VPrime;

  float intakeFlow{0.f};
  float exhaustFlow{0.f};
};

#endif

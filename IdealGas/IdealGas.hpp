#ifndef IDEALGAS_HPP
#define IDEALGAS_HPP

#include <valarray>

constexpr float DEFAULT_AMBIENT_PRESSURE = 101325.f;
constexpr float DEFAULT_AMBIENT_TEMPERATURE = 300.f;
constexpr float ZERO_CELSIUS_IN_KELVIN = 273.15f;
constexpr float PAToATM(float X) { return ((X) / DEFAULT_AMBIENT_PRESSURE); }
constexpr float KELVToCELS(float X) { return ((X)-ZERO_CELSIUS_IN_KELVIN); }

std::valarray<float> F(float t, std::valarray<float> &state, float VPrime);

class IdealGas {

public:
  float getP() { return state[0]; };
  float getV() { return state[1]; };
  float getnR() { return state[2]; };
  float getT() { return state[3]; };

  IdealGas(float p, float v, float t);

  static constexpr float alpha = 5.f / 2.f;
  std::valarray<float> state = {0.f, 0.f, 0.f, 0.f};
};

// class Gas : public IdealGas {

// public:
//   Gas(float p, float v, float t, float o);

//   float getOx() { return ox; };
//   float SimpleFlow(float kFlow, float ext_pressure, float ext_temp,
//                    float ext_ox, float dt);
//   void InjectHeat(float kx, float dt);

// private:
//   float ox; // Oxygenation level [0, 1]
// };

#endif

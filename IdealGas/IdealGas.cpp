#include "IdealGas.hpp"

IdealGas::IdealGas(float p, float v, float t) {
  pressure = p;
  volume = v;
  temperature = t;
  nR = pressure * volume / temperature;
}

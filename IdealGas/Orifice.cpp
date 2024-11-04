#include "Orifice.hpp"

#include <iostream>

Orifice::Orifice(float k_flow, Gas &gas1, Gas &gas2)
    : k_flow{k_flow}, gas1{gas1}, gas2{gas2} {
}

std::valarray<float> Orifice::flowThrough() {
  float nRPrime =
      k_flow * ((gas1.getP() > gas2.getP())
                    ? -gasFlowFunction(
                          gas1.getP(), gas2.getP(), gas1.getT(), gas2.getT())
                    : +gasFlowFunction(
                          gas2.getP(), gas1.getP(), gas2.getT(), gas1.getT()));

  // Consider the heat exchange given the flow
  const float QPrime =
      IDEALGAS_ALPHA * nRPrime * ((nRPrime > 0.f) ? gas2.getT() : gas1.getT());

  // TODO: Consider the heat diffusion through the orifice

  // Oxygenation
  const float k_ox = 50.f;
  const float oxPrime = (nRPrime > 0.f)
                            ? k_ox * nRPrime * (gas2.getOx() - gas1.getOx())
                            : k_ox * nRPrime * (gas1.getOx() - gas2.getOx());

  const float k_fuel = 100.0f;
  const float fuelPrime =
      (nRPrime < 0.f) ? -k_fuel * nRPrime * (gas2.getFuel() - gas1.getFuel())
                      : -k_fuel * nRPrime * (gas1.getFuel() - gas2.getFuel());

  // Create the derived state vector [V', nR', Q', ox', fuel']
  return std::valarray<float>{0.f, nRPrime, QPrime, oxPrime, fuelPrime};
}

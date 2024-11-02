#include "Orifice.hpp"

#include <iostream>

Orifice::Orifice(float k_flow, IdealGas &gas1, IdealGas &gas2)
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

  // Create the derived state vector [V', nR', Q']
  return std::valarray<float>{0.f, nRPrime, QPrime};
}

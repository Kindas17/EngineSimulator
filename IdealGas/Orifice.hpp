#ifndef ORIFICE_HPP
#define ORIFICE_HPP

#include "Gas.hpp"

class Orifice {
 public:
  Orifice(float k_flow, IdealGas &gas1, IdealGas &gas2);

  std::valarray<float> flowThrough();

 private:
  float k_flow{0};
  IdealGas &gas1;
  IdealGas &gas2;
};

#endif
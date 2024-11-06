#ifndef ORIFICE_HPP
#define ORIFICE_HPP

#include "Gas.hpp"

class Orifice {
 public:
  Orifice(float k_flow, Gas &gas1, Gas &gas2);

  std::valarray<float> flowThrough();

  void setKFlow(float new_k_flow) {
    k_flow = new_k_flow;
  }

 private:
  float k_flow{0};
  Gas &gas1;
  Gas &gas2;
};

#endif

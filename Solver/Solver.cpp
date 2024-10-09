#include "Solver.hpp"

#include <functional>
#include <valarray>

std::valarray<float> ForwardEuler(
    float deltaT,
    float t,
    std::valarray<float> &state,
    std::function<std::valarray<float>(float, std::valarray<float> &)> fun) {
  return state + deltaT * fun(t, state);
}

std::valarray<float> RungeKutta4(
    float deltaT,
    float t,
    std::valarray<float> &state,
    std::function<std::valarray<float>(float, std::valarray<float> &)> fun) {
  std::valarray<float> tmp;
  std::valarray<float> k1 = fun(t, state);

  tmp = state + deltaT * k1 / 2.f;
  std::valarray<float> k2 = fun(t + deltaT / 2.f, tmp);

  tmp = state + deltaT * k2 / 2.f;
  std::valarray<float> k3 = fun(t + deltaT / 2.f, tmp);

  tmp = state + deltaT * k3;
  std::valarray<float> k4 = fun(t + deltaT, tmp);

  return state + deltaT * (k1 + 2.f * k2 + 2.f * k3 + k4) / 6.f;
}

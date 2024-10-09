#ifndef SOLVER_HPP
#define SOLVER_HPP

#include <functional>

std::valarray<float> ForwardEuler(
    float deltaT,
    float t,
    std::valarray<float> &state,
    std::function<std::valarray<float>(float, std::valarray<float> &)> fun);

std::valarray<float> RungeKutta4(
    float deltaT,
    float t,
    std::valarray<float> &state,
    std::function<std::valarray<float>(float, std::valarray<float> &)> fun);

#endif

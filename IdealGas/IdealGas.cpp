#include "IdealGas.hpp"

#include <cmath>
#include <iostream>

#include "Geometry.hpp"

using namespace std::numbers;

constexpr float AIR_GAMMA = 1.4f;

IdealGas::IdealGas(float p, float v, float t) {
  state[0] = p;
  state[1] = v;
  state[3] = t;
  state[2] = state[0] * state[1] / state[3];
}

std::valarray<float> F_IdealGas(float t,
                                std::valarray<float> &st,
                                std::valarray<float> stp) {
  const float a = IdealGas::alpha;
  const float P = st[0];
  const float V = st[1];
  const float nR = st[2];
  const float T = st[3];

  const float Vp = stp[0];
  const float nRp = stp[1];
  const float Tp = (stp[2] - a * nRp * T - P * Vp) / (a * nR);
  const float Pp = (nRp * T + nR * Tp - P * Vp) / V;

  return std::valarray<float>{Pp, Vp, nRp, Tp};
}

void IdealGas::updateState(float kthermal,
                           float kFlow_int,
                           float kFlow_exh,
                           float Pout_int,
                           float Pout_exh,
                           float Tout_int,
                           float Tout_exh) {
  const float P = state[0];
  const float T = state[3];
  QPrime = 0.f;

  // Intake flow
  if (Pout_int > P) {
    intakeFlow = kFlow_int * gasFlowFunction(Pout_int, P, Tout_int, T);
  } else {
    intakeFlow = -kFlow_int * gasFlowFunction(P, Pout_int, T, Tout_int);
  }

  // Exhaust flow
  if (P > Pout_exh) {
    exhaustFlow = -kFlow_exh * gasFlowFunction(P, Pout_exh, T, Tout_exh);
  } else {
    exhaustFlow = kFlow_exh * gasFlowFunction(Pout_exh, P, Tout_exh, T);
  }

  nRPrime = intakeFlow + exhaustFlow;

  // Heat exchange: intake flow
  QPrime += (intakeFlow > 0.f) ? alpha * intakeFlow * Tout_int
                               : alpha * intakeFlow * T;

  // Heat exchange: exhaust flow
  QPrime += (exhaustFlow > 0.f) ? alpha * exhaustFlow * Tout_exh
                                : alpha * exhaustFlow * T;

  // Heat exchange: external world
  QPrime += kthermal * (DEFAULT_AMBIENT_TEMPERATURE - T);
}

std::valarray<float> IdealGas::exchangeHeat(float kTherm, float extTemp) {
  return std::valarray<float>{0.f, 0.f, kTherm * (extTemp - getT()), 0.f, 0.f};
}

float gasFlowFunction(float Pup, float Pdown, float Tup, float Tdown) {
  const float pDiff = Pdown / Pup;
  const float rhoUp = Pup / (IDEAL_GAS_CONSTANT * Tup);
  const float a = AIR_GAMMA / (AIR_GAMMA - 1);
  const float b = 2 * a * Pup * rhoUp;
  const float c =
      pow(pDiff, 2.f / AIR_GAMMA) - pow(pDiff, (AIR_GAMMA + 1) / AIR_GAMMA);

  return sqrt(b * c);
}

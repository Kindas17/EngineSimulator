#include "IdealGas.hpp"
#include <cmath>

using namespace std::numbers;

IdealGas::IdealGas(float p, float v, float t) {

  state[0] = p;
  state[1] = v;
  state[3] = t;
  state[2] = state[0] * state[1] / state[3];
}

std::valarray<float> F(float t, std::valarray<float> &st, float VPrime,
                       float nRPrime, float QPrime) {

  const float a = IdealGas::alpha;
  const float P = st[0];
  const float V = st[1];
  const float nR = st[2];
  const float T = st[3];

  const float nRp = nRPrime;
  const float Vp = VPrime;
  const float Tp = (QPrime - a * nRp * T - P * Vp) / (a * nR);
  const float Pp = (nRp * T + nR * Tp - P * Vp) / V;

  return std::valarray<float>{Pp, Vp, nRp, Tp};
}

void IdealGas::updateState(float Vp, float kFlow_int, float kFlow_exh,
                           float Pout_int, float Pout_exh, float Tout_int,
                           float Tout_exh) {

  const float P = state[0];
  const float T = state[3];
  VPrime = Vp;
  QPrime = 0.f;

  // Intake flow
  const float nRPrime_Int = kFlow_int * (Pout_int - P);

  // Exhaust flow
  const float nRPrime_Exh = kFlow_exh * (Pout_exh - P);
  nRPrime = nRPrime_Int + nRPrime_Exh;

  // Heat exchange: intake flow
  QPrime += (nRPrime_Int > 0.f) ? alpha * nRPrime_Int * Tout_int
                                : alpha * nRPrime_Int * T;

  // Heat exchange: exhaust flow
  QPrime += (nRPrime_Exh > 0.f) ? alpha * nRPrime_Exh * Tout_exh
                                : alpha * nRPrime_Exh * T;

  // Heat exchange: external world
  QPrime += 0.003f * (300.f - T);
}

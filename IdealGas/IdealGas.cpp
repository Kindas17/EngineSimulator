#include "IdealGas.hpp"
#include "Geometry.hpp"
#include <cmath>
#include <iostream>

using namespace std::numbers;

constexpr float IDEAL_GAS_CONSTANT = 8.314f;
constexpr float AIR_GAMMA = 1.4f;
// Approx molar mass of air in kg/mol
constexpr float M_air = 0.029f;
// Critical Pdown / Pup
constexpr float chokedFlowCondition = 1.f / 1.893f;

IdealGas::IdealGas(float p, float v, float t) {

  state[0] = p;
  state[1] = v;
  state[3] = t;
  state[2] = state[0] * state[1] / state[3];
}

std::valarray<float> F_IdealGas(float t, std::valarray<float> &st, float ang,
                                float omega, CylinderGeometry g, float nRPrime,
                                float QPrime) {

  const float a = IdealGas::alpha;
  const float P = st[0];
  const float V = st[1];
  const float nR = st[2];
  const float T = st[3];
  const float h = g.stroke;
  const float l = g.rod;
  const float r = g.bore * 0.5f;

  // VPrime computation
  const float k = std::numbers::pi * h * pow(r, 2);
  const float dx_1 = -h * cos(ang) * 0.5f;
  const float dx_2_num = -h * h * sin(ang) * cos(ang);
  const float dx_2_den = 4.f * l * sqrt(1.f - pow(h * cos(ang) * 0.5f / l, 2));
  const float dx_2 = dx_2_num / dx_2_den;
  const float dx = dx_1 + dx_2;
  const float dcperc = -dx / h;

  const float Vp = -k * dcperc * omega;
  const float nRp = nRPrime;
  const float Tp = (QPrime - a * nRp * T - P * Vp) / (a * nR);
  const float Pp = (nRp * T + nR * Tp - P * Vp) / V;

  return std::valarray<float>{Pp, Vp, nRp, Tp};
}

void IdealGas::updateState(float kthermal, float kFlow_int, float kFlow_exh,
                           float Pout_int, float Pout_exh, float Tout_int,
                           float Tout_exh) {

  const float P = state[0];
  const float T = state[3];
  QPrime = 0.f;

  // rho = P * M_air / (R * T)
  const float gamma = 1.4f;
  const float criticalPressureDiff = 0.528f;
  const float R = 8.314;     // Ideal gas constant
  const float M_air = 0.029; // Approx molar mass of air in kg/mol
  const float rho_intake = Pout_int * M_air / (R * Tout_int);
  const float rho_chamber = P * M_air / (R * T);
  const float rho_exhaust = Pout_exh * M_air / (R * Tout_exh);

  float squaredFlowFunction;
  float pDiff;

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
  QPrime += kthermal * (300.f - T);
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

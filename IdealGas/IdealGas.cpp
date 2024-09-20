#include "IdealGas.hpp"
#include "Geometry.hpp"
#include <cmath>

using namespace std::numbers;

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
  const float R = 8.314;     // Ideal gas constant
  const float M_air = 0.029; // Approx molar mass of air in kg/mol
  const float rho_intake = Pout_int * M_air / (R * Tout_int);
  const float rho_chamber = P * M_air / (R * T);
  const float rho_exhaust = Pout_exh * M_air / (R * Tout_exh);

  // Intake flow
  // intakeFlow = kFlow_int * (Pout_int - P);
  if (Pout_int > P) {
    intakeFlow = kFlow_int * sqrt(2 * (Pout_int - P) / rho_intake);
  } else {
    intakeFlow = -kFlow_int * sqrt(2 * (P - Pout_int) / rho_chamber);
  }

  // Exhaust flow
  // exhaustFlow = kFlow_exh * (Pout_exh - P);
  if (Pout_exh > P) {
    exhaustFlow = kFlow_exh * sqrt(2 * (Pout_exh - P) / rho_chamber);
  } else {
    exhaustFlow = -kFlow_exh * sqrt(2 * (P - Pout_exh) / rho_exhaust);
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

#include "Piston.hpp"
#include "Solver.hpp"
#include <cmath>
#include <functional>
#include <numbers>

using namespace std::placeholders;

static std::valarray<float> F_piston(float t, std::valarray<float> &st,
                                     float Ti, float Te);

inline float angleWrapper(float angle) {
  while (angle > 2.f * std::numbers::pi) {
    angle -= 2.f * std::numbers::pi;
  }
  while (angle < 0) {
    angle += 2.f * std::numbers::pi;
  }
  return angle;
}

static std::valarray<float> F_piston(float t, std::valarray<float> &st,
                                     float Ti, float Te) {

  const float omega = st[1];

  const float thetap = omega;
  const float omegap = Ti + Te;

  return std::valarray<float>{thetap, omegap};
}

Piston::Piston(CylinderGeometry geometryInfo)
    : externalTorque{}, combustionAdvance(0.f), combustionInProgress(false),
      intakeCoef(0.00012f), exhaustCoef(0.00008f), throttle(0.f),
      dynamicsIsActive(false), minThrottle(0.075f), ignitionOn(false) {

  geometry = geometryInfo;

  /* Dynamics */
  state = std::valarray<float>{DEGToRAD(25.f), 0.f};

  /* Initial update to initialize the piston status */
  rodFoot = {.x = +(geometry.stroke * 0.5f) * cos(getCurrentAngle()),
             .y = -(geometry.stroke * 0.5f) * sin(getCurrentAngle())};

  gas = new IdealGas(101325.f, getChamberVolume(), 300.f);
}

void Piston::update(float deltaT) {

  const float previousHeadAngle = getCurrentAngle();

  std::function<std::valarray<float>(float, std::valarray<float> &)> F2 =
      std::bind(F_piston, _1, _2, getTorque() / geometry.momentOfInertia,
                externalTorque / geometry.momentOfInertia);

  state = RungeKutta4(deltaT, 0.f, state, F2);
  state[0] = angleWrapper(state[0]);

  /* Update rod foot position */
  rodFoot = {.x = +(geometry.stroke / 2) * cos(getCurrentAngle()),
             .y = -(geometry.stroke / 2) * sin(getCurrentAngle())};

  /* A full cycle of the engine has terminated */
  if (previousHeadAngle > getCurrentAngle()) {
    cycleTrigger = true;
    combustionInProgress = false;
  }

  // Update valve position
  ValveMgm();

  // Update gas state
  gas->updateState(0.f, 0.f, 0.f, 101325.f, 101325.f, 300.f, 300.f);

  std::function<std::valarray<float>(float, std::valarray<float> &)> F3 =
      std::bind(F, _1, _2, getCurrentAngle(), state[1], geometry, 0.f, 0.f);

  gas->state = RungeKutta4(deltaT, 0.f, gas->state, F3);
}

void Piston::ValveMgm() {
  // /* Intake Profile */
  // const float profileSpeed1 = 30;
  // const float x_int =
  //     (angleWrapper(state[0] - 180) - (45 + 180)) / profileSpeed1;
  // intakeValve = expf(-(x_int * x_int));

  // /* Exhaust Profile */
  // const float profileSpeed2 = 20;
  // const float x_exh =
  //     (angleWrapper(state[0] + 180) - (315 - 180)) / profileSpeed2;
  // exhaustValve = expf(-(x_exh * x_exh));
}

float Piston::getPistonPosition() {
  const float a = rodFoot.y;
  const float b = pow(geometry.stroke * cos(getCurrentAngle()) * 0.5f, 2);
  const float c = b / pow(geometry.rod, 2);
  const float d = geometry.rod * sqrt(1 - c);
  return a - d;
}

float Piston::getCyclePercent() {
  return (-getPistonPosition() - geometry.rod + geometry.stroke * 0.5f) /
         geometry.stroke;
}

float Piston::getChamberVolume() {

  const float constantVol =
      std::numbers::pi * pow(geometry.bore * 0.5f, 2) * geometry.addStroke;

  return (1.f - getCyclePercent()) * std::numbers::pi * geometry.stroke *
             pow(geometry.bore * 0.5f, 2) +
         constantVol;
}

float Piston::getMaxVolume() {

  return std::numbers::pi * pow(geometry.bore * 0.5f, 2) *
         (geometry.addStroke + geometry.stroke);
}

float Piston::getEngineVolume() {

  return std::numbers::pi * pow(geometry.bore * 0.5f, 2) * geometry.stroke;
}

float Piston::getCompressionRatio() {

  return getMaxVolume() /
         (pow(geometry.bore * 0.5f, 2) * std::numbers::pi * geometry.addStroke);
}

float Piston::getCurrentAngle() {
  return angleWrapper(state[0] * 2.f + std::numbers::pi * 0.5f);
}

float Piston::getThetaAngle() {

  return asin(geometry.stroke / (2.f * geometry.rod) * cos(getCurrentAngle()));
}

float Piston::getTorque() {

  const float pistonSurface = std::numbers::pi * pow(geometry.bore * 0.5f, 2);
  const float topPistonPressure = gas->getP();

  const float force =
      pistonSurface * (topPistonPressure - 101325.f) * cos(getThetaAngle());

  const float friction = -state[1] * 0.05f;
  const float absTorque = (force * geometry.stroke) / 2.f;

  return (getThetaAngle() < 0.f) ? absTorque + friction : -absTorque + friction;
}

constexpr float Piston::getThrottle(float curr) {

  return (1.f - minThrottle) * curr + minThrottle;
}

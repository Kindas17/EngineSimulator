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
    : externalTorque{}, combustionInProgress(false), throttle(0.f),
      dynamicsIsActive(false) {

  geometry = geometryInfo;

  /* Dynamics */
  state = std::valarray<float>{DEGToRAD(0.f), 0.f};

  /* Initial update to initialize the piston status */
  rodFoot = {.x = +(geometry.stroke * 0.5f) * cos(getCurrentAngle()),
             .y = -(geometry.stroke * 0.5f) * sin(getCurrentAngle())};

  gas = new Gas(DEFAULT_AMBIENT_PRESSURE, getChamberVolume(),
                DEFAULT_AMBIENT_TEMPERATURE, 1.f);
  intakeGas = new Gas(3 * DEFAULT_AMBIENT_PRESSURE, 20.f * getChamberVolume(),
                      DEFAULT_AMBIENT_TEMPERATURE, 1.f);
  exhaustGas = new Gas(3 * DEFAULT_AMBIENT_PRESSURE, 20.f * getChamberVolume(),
                       DEFAULT_AMBIENT_TEMPERATURE, 1.f);

  ignitionOn = true;
}

Piston::Piston(CylinderGeometry geometryInfo, float omega0)
    : externalTorque{}, combustionInProgress(false), throttle(0.f),
      dynamicsIsActive(false) {

  geometry = geometryInfo;

  /* Dynamics */
  state = std::valarray<float>{DEGToRAD(0.f), omega0};
  killDynamics = true;

  /* Initial update to initialize the piston status */
  rodFoot = {.x = +(geometry.stroke * 0.5f) * cos(getCurrentAngle()),
             .y = -(geometry.stroke * 0.5f) * sin(getCurrentAngle())};

  gas = new Gas(DEFAULT_AMBIENT_PRESSURE, getChamberVolume(),
                DEFAULT_AMBIENT_TEMPERATURE, 1.f);
  intakeGas = new Gas(DEFAULT_AMBIENT_PRESSURE, 20.f * getChamberVolume(),
                      DEFAULT_AMBIENT_TEMPERATURE, 1.f);
  exhaustGas = new Gas(DEFAULT_AMBIENT_PRESSURE, 20.f * getChamberVolume(),
                       DEFAULT_AMBIENT_TEMPERATURE, 1.f);

  ignitionOn = true;
}

void Piston::update(float deltaT) {

  const float previousHeadAngle = getHeadAngle();

  std::function<std::valarray<float>(float, std::valarray<float> &)> F2 =
      std::bind(F_piston, _1, _2,
                killDynamics ? 0.0f : (getTorque() / geometry.momentOfInertia),
                killDynamics ? 0.0f
                             : (externalTorque / geometry.momentOfInertia));

  state = RungeKutta4(deltaT, 0.f, state, F2);
  state[0] = angleWrapper(state[0]);

  /* Update rod foot position */
  rodFoot = {.x = +(geometry.stroke * 0.5f) * cos(getCurrentAngle()),
             .y = -(geometry.stroke * 0.5f) * sin(getCurrentAngle())};

  // Update valve position
  ValveMgm();

  // Update gas state
  gas->updateState(kthermal, getThrottle(throttle) * intakeValve * intakeCoef,
                   exhaustCoef * exhaustValve, intakeGas->getP(),
                   exhaustGas->getP(), intakeGas->getT(), exhaustGas->getT(),
                   intakeGas->getOx(), exhaustGas->getOx(),
                   combustionInProgress ? combustionSpeed : 0.f,
                   combustionEnergy);
  intakeGas->updateState(
      kthermal, 0.001f, getThrottle(throttle) * intakeValve * intakeCoef,
      DEFAULT_AMBIENT_PRESSURE, gas->getP(), DEFAULT_AMBIENT_TEMPERATURE,
      gas->getT(), 1.f, gas->getOx(), 0.f, 0.f);
  exhaustGas->updateState(kthermal, 0.001f, exhaustCoef * exhaustValve,
                          DEFAULT_AMBIENT_PRESSURE, gas->getP(),
                          DEFAULT_AMBIENT_TEMPERATURE, gas->getT(), 1.f,
                          gas->getOx(), 0.f, 0.f);

  std::function<std::valarray<float>(float, std::valarray<float> &)> F3 =
      std::bind(F_Gas, _1, _2, getCurrentAngle(), getEngineSpeed(), geometry,
                gas->nRPrime, gas->QPrime, gas->oxPrime);
  std::function<std::valarray<float>(float, std::valarray<float> &)> F4 =
      std::bind(F_Gas, _1, _2, 0.f, 0.f, geometry, intakeGas->nRPrime,
                intakeGas->QPrime, intakeGas->oxPrime);
  std::function<std::valarray<float>(float, std::valarray<float> &)> F5 =
      std::bind(F_Gas, _1, _2, 0.f, 0.f, geometry, exhaustGas->nRPrime,
                exhaustGas->QPrime, exhaustGas->oxPrime);

  gas->state = RungeKutta4(deltaT, 0.f, gas->state, F3);
  intakeGas->state = RungeKutta4(deltaT, 0.f, intakeGas->state, F4);
  exhaustGas->state = RungeKutta4(deltaT, 0.f, exhaustGas->state, F5);

  // Spark plug event
  if (ignitionOn &&
      getHeadAngle() > std::numbers::pi - DEGToRAD(combustionAdvance)) {
    combustionInProgress = true;
  }

  /* A full cycle of the engine has terminated */
  if (previousHeadAngle > getHeadAngle()) {
    cycleTrigger = true;
    combustionInProgress = false;
  }
}

void Piston::ValveMgm() {
  using namespace std::numbers;

  /* Intake Profile */
  const float profileSpeed1 = DEGToRAD(intakeShape);
  const float x_int =
      (angleWrapper(state[0] - pi) - (DEGToRAD(intakeTiming) + pi)) /
      profileSpeed1;
  intakeValve = expf(-(x_int * x_int));

  /* Exhaust Profile */
  const float profileSpeed2 = DEGToRAD(exhaustShape);
  const float x_exh =
      (angleWrapper(state[0] + pi) - (DEGToRAD(exhaustTiming) - pi)) /
      profileSpeed2;
  exhaustValve = expf(-(x_exh * x_exh));
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

float Piston::getHeadAngle() { return state[0]; }

float Piston::getThetaAngle() {

  return asin(geometry.stroke / (2.f * geometry.rod) * cos(getCurrentAngle()));
}

float Piston::getTorque() {

  const float pistonSurface = std::numbers::pi * pow(geometry.bore * 0.5f, 2);
  const float topPistonPressure = gas->getP();

  const float force =
      pistonSurface * (topPistonPressure - 101325.f) * cos(getThetaAngle());

  const float friction = -state[1] * 0.01f;
  const float absTorque = (force * geometry.stroke) / 2.f;

  return (getThetaAngle() < 0.f) ? absTorque + friction : -absTorque + friction;
}

float Piston::getEngineSpeed() { return state[1] * 2.f; }

constexpr float Piston::getThrottle(float curr) {

  return (1.f - minThrottle) * curr + minThrottle;
}

void Piston::setEngineSpeed(float omega) { state[1] = omega * 0.5f; }
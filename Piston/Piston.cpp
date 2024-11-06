#include "Piston.hpp"

#include <cmath>
#include <functional>
#include <iostream>
#include <numbers>

#include "Solver.hpp"

using namespace std::placeholders;

static std::valarray<float> F_piston(float t,
                                     std::valarray<float> &st,
                                     float Ti,
                                     float Te);

inline float angleWrapper(float angle) {
  while (angle > 2.f * std::numbers::pi) {
    angle -= 2.f * std::numbers::pi;
  }
  while (angle < 0) {
    angle += 2.f * std::numbers::pi;
  }
  return angle;
}

static std::valarray<float> F_piston(float t,
                                     std::valarray<float> &st,
                                     float Ti,
                                     float Te) {
  return std::valarray<float>{st[1], Ti + Te};
}

Piston::Piston(EngineConfig engineCfg)
    : cfg(engineCfg),
      combustionInProgress(false),
      throttle(0.f),
      dynamicsIsActive(false),
      intakeValveOrif(Orifice(0.f, gas, intakeManifold)),
      exhaustValveOrif(Orifice(0.f, gas, exhaustPipe)) {
  /* Dynamics */
  state = std::valarray<float>{DEGToRAD(0.f), 0.f};

  /* Thermodynamics */
  gas = Gas(DEFAULT_AMBIENT_PRESSURE,
            getChamberVolume(),
            DEFAULT_AMBIENT_TEMPERATURE,
            0.f);
  intakeManifold =
      Gas(DEFAULT_AMBIENT_PRESSURE, 1000.f, DEFAULT_AMBIENT_TEMPERATURE, 1.f);

  exhaustPipe =
      Gas(DEFAULT_AMBIENT_PRESSURE, 1000.f, DEFAULT_AMBIENT_TEMPERATURE, 0.f);

  /* Initial update to initialize the piston status */
  rodFoot = std::valarray<float>{
      +(cfg.cylinder.stroke * 0.5f) * cosf(getCurrentAngle()),
      -(cfg.cylinder.stroke * 0.5f) * sinf(getCurrentAngle())};

  ignitionOn = true;
}

void Piston::update(float deltaT) {
  const float previousHeadAngle = getHeadAngle();

  std::function<std::valarray<float>(float, std::valarray<float> &)> F2 =
      std::bind(F_piston,
                _1,
                _2,
                killDynamics ? 0.0f : (getTorque() / cfg.momentOfInertia),
                killDynamics ? 0.0f : (externalTorque / cfg.momentOfInertia));

  state = RungeKutta4(deltaT, 0.f, state, F2);
  state[0] = angleWrapper(state[0]);

  /* Update rod foot position */
  rodFoot = std::valarray<float>{
      +(cfg.cylinder.stroke * 0.5f) * cosf(getCurrentAngle()),
      -(cfg.cylinder.stroke * 0.5f) * sinf(getCurrentAngle())};

  // Update valve position
  ValveMgm();

  intakeValveOrif.setKFlow(getThrottle(throttle) * intakeValve * intakeCoef);
  const auto stp1_int = intakeValveOrif.flowThrough();
  const auto stp2_int = chamberDisplacement();
  const auto stp_int = stp1_int + stp2_int;
  intakeFlow = stp_int[1];
  gas.state = RungeKutta4(
      deltaT,
      0.f,
      gas.state,
      std::bind(F_Gas,
                std::placeholders::_1,
                std::placeholders::_2,
                stp_int + gas.exchangeHeat(1.f, DEFAULT_AMBIENT_TEMPERATURE)));

  exhaustValveOrif.setKFlow(exhaustValve * exhaustCoef);
  const auto stp1_exh = exhaustValveOrif.flowThrough();
  const auto stp2_exh = chamberDisplacement();
  const auto stp_exh = stp1_exh + stp2_exh;
  exhaustFlow = stp_exh[1];
  gas.state = RungeKutta4(
      deltaT,
      0.f,
      gas.state,
      std::bind(F_Gas,
                std::placeholders::_1,
                std::placeholders::_2,
                stp_exh + gas.exchangeHeat(3.f, DEFAULT_AMBIENT_TEMPERATURE)));

  auto stp_comb = gas.combust(
      combustionInProgress ? combustionSpeed : 0.f, combustionEnergy);
  gas.state = RungeKutta4(
      deltaT,
      0.f,
      gas.state,
      std::bind(F_Gas, std::placeholders::_1, std::placeholders::_2, stp_comb));

  // Spark plug event
  if (ignitionOn &&
      getHeadAngle() > std::numbers::pi - DEGToRAD(combustionAdvance)) {
    if (!combustionInProgress) {
      gas.setFuelAmnt(14.7f);
      totalFuelConsumption += gas.fuelInjected;
    }
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
  const float profileSpeed1 = DEGToRAD(cfg.intakeValve.shape);
  const float x_int =
      (angleWrapper(state[0] - pi) - (DEGToRAD(cfg.intakeValve.timing) + pi)) /
      profileSpeed1;
  intakeValve = expf(-(x_int * x_int));

  /* Exhaust Profile */
  const float profileSpeed2 = DEGToRAD(cfg.exhaustValve.shape);
  const float x_exh =
      (angleWrapper(state[0] + pi) - (DEGToRAD(cfg.exhaustValve.timing) - pi)) /
      profileSpeed2;
  exhaustValve = expf(-(x_exh * x_exh));
}

float Piston::getPistonPosition() {
  const float a = rodFoot[1];
  const float b = pow(cfg.cylinder.stroke * cos(getCurrentAngle()) * 0.5f, 2);
  const float c = b / pow(cfg.rod.length, 2);
  const float d = cfg.rod.length * sqrt(1 - c);
  return a - d;
}

float Piston::getCyclePercent() {
  return (-getPistonPosition() - cfg.rod.length + cfg.cylinder.stroke * 0.5f) /
         cfg.cylinder.stroke;
}

float Piston::getChamberVolume() {
  const float constantVol = std::numbers::pi *
                            pow(cfg.cylinder.bore * 0.5f, 2) *
                            cfg.cylinder.add_stroke;

  return (1.f - getCyclePercent()) * std::numbers::pi * cfg.cylinder.stroke *
             pow(cfg.cylinder.bore * 0.5f, 2) +
         constantVol;
}

float Piston::getMaxVolume() {
  return std::numbers::pi * pow(cfg.cylinder.bore * 0.5f, 2) *
         (cfg.cylinder.add_stroke + cfg.cylinder.stroke);
}

float Piston::getEngineVolume() {
  return std::numbers::pi * pow(cfg.cylinder.bore * 0.5f, 2) *
         cfg.cylinder.stroke;
}

float Piston::getCompressionRatio() {
  return getMaxVolume() / (pow(cfg.cylinder.bore * 0.5f, 2) * std::numbers::pi *
                           cfg.cylinder.add_stroke);
}

float Piston::getCurrentAngle() {
  return angleWrapper(state[0] * 2.f + std::numbers::pi * 0.5f);
}

float Piston::getHeadAngle() {
  return state[0];
}

float Piston::getThetaAngle() {
  return asin(cfg.cylinder.stroke / (2.f * cfg.rod.length) *
              cos(getCurrentAngle()));
}

float Piston::getTorque() {
  const float pistonSurface =
      std::numbers::pi * pow(cfg.cylinder.bore * 0.5f, 2);
  const float topPistonPressure = gas.getP();

  const float force =
      pistonSurface * (topPistonPressure - 101325.f) * cos(getThetaAngle());

  const float friction = -state[1] * 0.01f;
  const float absTorque = (force * cfg.cylinder.stroke) / 2.f;

  return (getThetaAngle() < 0.f) ? absTorque + friction : -absTorque + friction;
}

float Piston::getEngineSpeed() {
  return state[1] * 2.f;
}

constexpr float Piston::getThrottle(float curr) {
  return (1.f - minThrottle) * curr + minThrottle;
}

void Piston::setEngineSpeed(float omega) {
  state[1] = omega * 0.5f;
}

std::valarray<float> Piston::chamberDisplacement() {
  const float h = cfg.cylinder.stroke;
  const float l = cfg.rod.length;
  const float r = cfg.cylinder.bore * 0.5f;
  const float ang = getCurrentAngle();
  const float omega = getEngineSpeed();

  // VPrime computation
  const float k = std::numbers::pi * h * pow(r, 2);
  const float dx_1 = -h * cos(ang) * 0.5f;
  const float dx_2_num = -h * h * sin(ang) * cos(ang);
  const float dx_2_den = 4.f * l * sqrt(1.f - pow(h * cos(ang) * 0.5f / l, 2));
  const float dx_2 = dx_2_num / dx_2_den;
  const float dx = dx_1 + dx_2;
  const float dcperc = -dx / h;

  return std::valarray<float>{-k * dcperc * omega, 0.f, 0.f, 0.f, 0.f};
}

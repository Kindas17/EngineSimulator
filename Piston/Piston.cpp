#include "Piston.hpp"
#include "Solver.hpp"
#include <cmath>
#include <numbers>
#include <functional>

using namespace std::placeholders;

constexpr float CRANKSHAFT_L = 25.0f;            /* [mm] */
constexpr float BIELLA_L = 55.0f;                /* [mm] */
constexpr float ADD_STROKE = CRANKSHAFT_L / 3.f; /* No Unit */

inline float angleWrapper(float angle) {
  while (angle > 360) {
    angle -= 360.0;
  }
  while (angle < 0) {
    angle += 360;
  }
  return angle;
}

Piston::Piston(CylinderGeometry geometryInfo) : omega{}, externalTorque{} {
  /* Piston Geometry */
  this->geometry = geometryInfo;

  /* Dynamics */
  headAngle = 35;
  currentAngle = headAngle * 2 + 90; /* Deg */

  minThrottle = 0.075f;
  throttle = 0.f;

  ignitionOn = false;
  combustionInProgress = false;
  combustionAdvance = 0.f;
  kexpl = 0.07f;

  // Calibrations
  thermalK = 0.5f;

  // Valves
  intakeCoef = 0.000012f;
  exhaustCoef = 0.000008f;

  /* Initial update to initialize the piston status */
  rodFoot = {.x = +(geometry.stroke / 2) * cosf(DEGToRAD(currentAngle)),
             .y = -(geometry.stroke / 2) * sinf(DEGToRAD(currentAngle))};

  /* Thermodynamics */
  gas = new IdealGas(101325.f, getChamberVolume(), 300.f);

  dynamicsIsActive = false;
}

void Piston::updatePosition(float deltaT, float setSpeed) {
  const float previousHeadAngle = headAngle;
  headAngle +=
      RADToDEG(omega) * deltaT / 2; /* Head crank rotates at half the speed */

  currentAngle = headAngle * 2 + 90;

  /* Angle Wrapping */
  currentAngle = angleWrapper(currentAngle);
  headAngle = angleWrapper(headAngle);

  const float prevV = getChamberVolume();

  /* Update rod foot position */
  rodFoot = {.x = +(geometry.stroke / 2) * cosf(DEGToRAD(currentAngle)),
             .y = -(geometry.stroke / 2) * sinf(DEGToRAD(currentAngle))};

  V_prime = (getChamberVolume() - prevV) / deltaT;

  if (this->dynamicsIsActive) {
    omega += deltaT * (getTorque() + externalTorque) / geometry.momentOfInertia;
  } else {
    omega = setSpeed;
  }

  /* Check if the spark plug has triggered */
  if (ignitionOn && previousHeadAngle < combustionAdvance + 180 &&
      headAngle > combustionAdvance + 180) {
    combustionInProgress = true;
  }

  /* A full cycle of the engine has terminated */
  if (previousHeadAngle > headAngle) {
    cycleTrigger = true;
    combustionInProgress = false;
  }

  updateStatus(deltaT);
}

void Piston::updateStatus(float deltaT) {

  // Update valve position
  ValveMgm();

  gas->updateState(V_prime, intakeCoef * intakeValve,
                   exhaustCoef * exhaustValve, 101325.f, 101325.f, 300.f,
                   300.f);

  std::function<std::valarray<float>(float, std::valarray<float> &)> F2 =
      std::bind(F, _1, _2, gas->VPrime, gas->nRPrime, gas->QPrime);

  gas->state = RungeKutta4(deltaT, 0.f, gas->state, F2);
}

void Piston::applyExtTorque(float torque) { externalTorque = torque; }

void Piston::ValveMgm() {
  /* Intake Profile */
  const float profileSpeed1 = 30;
  const float x_int =
      (angleWrapper(headAngle - 180) - (45 + 180)) / profileSpeed1;
  intakeValve = expf(-(x_int * x_int));

  /* Exhaust Profile */
  const float profileSpeed2 = 20;
  const float x_exh =
      (angleWrapper(headAngle + 180) - (315 - 180)) / profileSpeed2;
  exhaustValve = expf(-(x_exh * x_exh));
}

float Piston::getPistonPosition() {
  const float a = rodFoot.y;
  const float b = (geometry.stroke / 2) * (geometry.stroke / 2) *
                  cosf(DEGToRAD(currentAngle)) * cosf(DEGToRAD(currentAngle));
  const float c = b / (geometry.rod * geometry.rod);
  const float d = geometry.rod * sqrtf(1 - c);
  return a - d;
}

float Piston::getCyclePercent() {
  return (-getPistonPosition() - geometry.rod + geometry.stroke / 2) /
         (geometry.stroke);
}

float Piston::getChamberVolume() {
  const float constantVol = geometry.bore * geometry.bore * std::numbers::pi *
                            0.25 * geometry.addStroke;
  return (1 - getCyclePercent()) * std::numbers::pi * geometry.bore *
             geometry.bore * geometry.stroke * 0.25 +
         constantVol;
}

float Piston::getMaxVolume() {
  return std::numbers::pi * (geometry.bore) * (geometry.bore) *
         (geometry.addStroke + geometry.stroke) * 0.25;
}

float Piston::getEngineVolume() {
  return std::numbers::pi * (geometry.bore) * (geometry.bore) *
         (geometry.stroke) * 0.25;
}

float Piston::getCompressionRatio() {
  return getMaxVolume() / (geometry.bore * geometry.bore * std::numbers::pi *
                           0.25 * geometry.addStroke);
}

float Piston::getThetaAngle() {
  return asinf(geometry.stroke / (2 * geometry.rod) *
               cosf(DEGToRAD(currentAngle)));
}

float Piston::getTorque() {
  // const float pistonSurface =
  //     (geometry.bore * geometry.bore * std::numbers::pi * 0.25);
  // const float topPistonPressure = gas->getP(); // thermo.gas.P;
  // const float force =
  //     pistonSurface * (topPistonPressure - 101325) *
  //     std::cos(getThetaAngle());

  // const float friction = -omega * 0.05f;
  // const float absTorque = (force * geometry.stroke) / 2;

  // return (getThetaAngle() < 0) ? absTorque + friction : -absTorque +
  // friction;
  return 0.f;
}

constexpr float Piston::getThrottle(float curr) {
  return (1.f - minThrottle) * curr + minThrottle;
}

CylinderGeometry::CylinderGeometry() {
  stroke = MMToM(CRANKSHAFT_L * 2.f);
  addStroke = MMToM(ADD_STROKE);
  rod = MMToM(BIELLA_L);
  bore = MMToM(50.6);
  momentOfInertia = 25 * (stroke / 2) * (stroke / 2);
}

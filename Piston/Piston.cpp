#include "Piston.hpp"
#include <cmath>
#include <numbers>

#define CRANKSHAFT_L (25.0)           /* [mm] */
#define BIELLA_L (55.0)               /* [mm] */
#define ADD_STROKE (CRANKSHAFT_L / 3) /* No Unit */

inline float angleWrapper(float angle) {
  while (angle > 360) {
    angle -= 360.0;
  }
  while (angle < 0) {
    angle += 360;
  }
  return angle;
}

inline float adiabaticCompression(float initialVolume, float initialPressure,
                                  float currentVolume) {
  return initialPressure * powf((initialVolume / currentVolume), 7.0 / 5.0);
}

Piston::Piston(CylinderGeometry geometryInfo) : omega{}, headAngle{}, externalTorque{} {
  /* Piston Geometry */
  this->geometry = geometryInfo;

  /* Dynamics */
  currentAngle = headAngle * 2 + 90; /* Deg */

  minThrottle = 0.0075;
  throttle = 0;

  ignitionOn = false;
  combustionAdvance = 7.5;

  /* Initial update to initialize the piston status */
  rodFoot = {.x = +(geometry.stroke / 2) * cosf(DEGToRAD(currentAngle)),
             .y = -(geometry.stroke / 2) * sinf(DEGToRAD(currentAngle))};

  /* Thermodynamics */
  gas = new IdealGas(DEFAULT_AMBIENT_PRESSURE, getChamberVolume(), 300.f);

  dynamicsIsActive = true;
}

void Piston::updatePosition(float deltaT, float setSpeed) {
  const float previousHeadAngle = headAngle;
  headAngle +=
      RADToDEG(omega) * deltaT / 2; /* Head crank rotates at half the speed */

  currentAngle = headAngle * 2 + 90;

  /* Angle Wrapping */
  currentAngle = angleWrapper(currentAngle);
  headAngle = angleWrapper(headAngle);

  if (previousHeadAngle > headAngle) {
    cycleTrigger = true;
  }

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

  updateStatus(deltaT);
}

void Piston::updateStatus(float deltaT) {
  /* Valve Status Update */
  ValveMgm();

  /* Thermodynamics */
  gas->AdiabaticCompress(V_prime, deltaT);
  intakeFlow = gas->SimpleFlow(0.0001f * intakeValve, DEFAULT_AMBIENT_PRESSURE,
                               DEFAULT_AMBIENT_TEMPERATURE, deltaT);
  exhaustFlow =
      gas->SimpleFlow(0.0001f * exhaustValve, DEFAULT_AMBIENT_PRESSURE,
                      DEFAULT_AMBIENT_TEMPERATURE, deltaT);
  gas->HeatExchange(0.05f, DEFAULT_AMBIENT_TEMPERATURE, deltaT);
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
  const float constantVol =
      geometry.bore * geometry.bore * std::numbers::pi * 0.25 * geometry.addStroke;
  return (1 - getCyclePercent()) * std::numbers::pi * geometry.bore * geometry.bore *
             geometry.stroke * 0.25 +
         constantVol;
}

float Piston::getMaxVolume() {
  return std::numbers::pi * (geometry.bore) * (geometry.bore) *
         (geometry.addStroke + geometry.stroke) * 0.25;
}

float Piston::getEngineVolume() {
  return std::numbers::pi * (geometry.bore) * (geometry.bore) * (geometry.stroke) * 0.25;
}

float Piston::getCompressionRatio() {
  return getMaxVolume() /
         (geometry.bore * geometry.bore * std::numbers::pi * 0.25 * geometry.addStroke);
}

float Piston::getThetaAngle() {
  return asinf(geometry.stroke / (2 * geometry.rod) *
               cosf(DEGToRAD(currentAngle)));
}

float Piston::getTorque() {
  const float pistonSurface = (geometry.bore * geometry.bore * std::numbers::pi * 0.25);
  const float topPistonPressure = gas->getP(); // thermo.gas.P;
  const float force =
      pistonSurface * (topPistonPressure - 101325) * std::cos(getThetaAngle());

  const float friction = -omega * 0.05f;
  const float absTorque = (force * geometry.stroke) / 2;

  return (getThetaAngle() < 0) ? absTorque + friction : -absTorque + friction;
  return 0.f;
}

CylinderGeometry::CylinderGeometry() {
  stroke = MM_TO_M(CRANKSHAFT_L * 2);
  addStroke = MM_TO_M(ADD_STROKE);
  rod = MM_TO_M(BIELLA_L);
  bore = MM_TO_M(50.6);
  momentOfInertia = 25 * (stroke / 2) * (stroke / 2);
}

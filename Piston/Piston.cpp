#include "Piston.hpp"

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

Piston::Piston(CylinderGeometry geometryInfo) {
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
  gas->SimpleFlow(0.0001f * intakeValve, DEFAULT_AMBIENT_PRESSURE, deltaT);
  gas->SimpleFlow(0.0001f * exhaustValve, DEFAULT_AMBIENT_PRESSURE, deltaT);

  // /* Combustion advance adjusting */
  // // combustionAdvance = (RADSToRPM(omega) < 2500) ? 7.5 * RADSToRPM(omega) /
  // 2500 : 7.5;

  // const float intake_k_flow = intakeValve * (throttle * (1 - minThrottle) +
  // minThrottle);

  // intakeFlow = intakeOri.flow(intake_k_flow * 0.2f * thermo.gas.nR,
  // thermo.gas.P); exhaustFlow = exhaustOri.flow(exhaustValve * 0.2f *
  // thermo.gas.nR, thermo.gas.P); const float nR_prime = intakeFlow +
  // exhaustFlow;

  // thermo.update(deltaT, nR_prime, V_prime, 300, 0);

  // // /* Thermodynamics Status Update */
  // // thermo.adiabatic(getChamberVolume());

  // // const float coef = 40.0f;
  // // const float throttleCoef = ((1-minThrottle)*throttle + minThrottle);
  // // const float intake_k_flow = intakeValve * throttleCoef * coef;

  // // intakeFlow = thermo.orificeFlow(deltaT, intake_k_flow, 0.25, 300,
  // 101325, 1);
  // // exhaustFlow = thermo.orificeFlow(deltaT, exhaustValve * coef, 0.25, 300,
  // 101325, 0);
  // // leakageFlow = thermo.orificeFlow(deltaT, 0.00001f * coef, 0.01, 300,
  // 101325, 0);

  // if (ignitionOn && (headAngle >= 180-combustionAdvance))
  // {
  //     // thermo.combustion(deltaT, internalTime);
  //     internalTime += deltaT;
  // } else {
  //     internalTime = 0;
  // }
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
      geometry.bore * geometry.bore * M_PI * 0.25 * geometry.addStroke;
  return (1 - getCyclePercent()) * M_PI * geometry.bore * geometry.bore *
             geometry.stroke * 0.25 +
         constantVol;
}

float Piston::getMaxVolume() {
  return M_PI * (geometry.bore) * (geometry.bore) *
         (geometry.addStroke + geometry.stroke) * 0.25;
}

float Piston::getEngineVolume() {
  return M_PI * (geometry.bore) * (geometry.bore) * (geometry.stroke) * 0.25;
}

float Piston::getCompressionRatio() {
  return getMaxVolume() /
         (geometry.bore * geometry.bore * M_PI * 0.25 * geometry.addStroke);
}

float Piston::getThetaAngle() {
  return asinf(geometry.stroke / (2 * geometry.rod) *
               cosf(DEGToRAD(currentAngle)));
}

float Piston::getTorque() {
  const float pistonSurface = (geometry.bore * geometry.bore * M_PI * 0.25);
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

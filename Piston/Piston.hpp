#ifndef PISTON_HPP
#define PISTON_HPP
#include "IdealGas.hpp"
#include "Linalg.hpp"
#include <numbers>
#include <stdio.h>

constexpr float DEGToRAD(float X) {
  return (2.0 * std::numbers::pi * (X) / 360.0);
}
constexpr float RADToDEG(float X) {
  return (360.0 * (X) / (2 * std::numbers::pi));
}
constexpr float RADSToHZ(float X) { return ((X) / (2 * std::numbers::pi)); }
constexpr float RADSToRPM(float X) { return (60.0 * RADSToHZ((X))); }
constexpr float RPMToHz(float X) { return X / 60.f; }
constexpr float RPMToRADS(float X) {
  return 2.f * std::numbers::pi * RPMToHz(X);
}
constexpr float M3ToCC(float X) { return (1000000 * (X)); }
constexpr float MMToM(float X) { return ((X) / 1000.0); }

class CylinderGeometry {
public:
  CylinderGeometry();
  float bore;
  float rod;
  float stroke;
  float addStroke;
  float momentOfInertia;
};

class Piston {
public:
  Piston(CylinderGeometry geometryInfo);

  /* Internal Clock */
  float internalTime;

  std::valarray<float> state = {0.f, 0.f}; // Head angle, omega

  /* Specs */
  CylinderGeometry geometry;
  vector2_T rodFoot;

  float throttle;
  float minThrottle;

  /* Dynamics */
  bool ignitionOn;
  float currentAngle;
  void updatePosition(float deltaT);
  void updateStatus(float deltaT);
  void ValveMgm();
  void applyExtTorque(float torque);
  float externalTorque;

  float combustionAdvance;

  /* Generic Methods */
  float getPistonPosition();
  float getCyclePercent();
  float getChamberVolume();
  float getMaxVolume();
  float getEngineVolume();
  float getCompressionRatio();
  float getThetaAngle();
  float getTorque();
  constexpr float getThrottle(float curr);

  /* Thermodynamics */
  float V_prime;
  IdealGas *gas;
  bool combustionInProgress;
  float kexpl;

  /* Valves */
  float intakeValve;
  float exhaustValve;
  float intakeFlow;
  float exhaustFlow;
  float leakageFlow;
  float intakeCoef;
  float exhaustCoef;

  float thermalK;

  /* Settings */
  bool dynamicsIsActive;

  /* Triggers */
  bool cycleTrigger;

private:
};

#endif

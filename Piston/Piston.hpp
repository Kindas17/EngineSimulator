#ifndef PISTON_HPP
#define PISTON_HPP
#include "Geometry.hpp"
#include "IdealGas.hpp"
#include "Linalg.hpp"
#include <numbers>

constexpr float DEGToRAD(float X) {
  return (2.0 * std::numbers::pi * (X) / 360.f);
}
constexpr float RADToDEG(float X) {
  return (360.0 * (X) / (2 * std::numbers::pi));
}
constexpr float RADSToHZ(float X) { return ((X) / (2.f * std::numbers::pi)); }
constexpr float RADSToRPM(float X) { return (60.f * RADSToHZ((X))); }
constexpr float RPMToHz(float X) { return X / 60.f; }
constexpr float RPMToRADS(float X) {
  return 2.f * std::numbers::pi * RPMToHz(X);
}

class Piston {
public:
  Piston(CylinderGeometry geometryInfo);

  /* Internal Clock */
  float internalTime{};

  std::valarray<float> state = {0.f, 0.f}; // Head angle, omega

  /* Specs */
  CylinderGeometry geometry;
  vector2_T rodFoot{};

  float throttle;
  float minThrottle;

  /* Dynamics */
  bool ignitionOn;
  void update(float deltaT);
  void ValveMgm();
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
  float getCurrentAngle();
  float getTorque();
  constexpr float getThrottle(float curr);

  /* Thermodynamics */
  IdealGas *gas;
  bool combustionInProgress;

  /* Valves */
  float intakeValve{};
  float exhaustValve{};
  float intakeFlow{};
  float exhaustFlow{};
  float leakageFlow{};
  float intakeCoef;
  float exhaustCoef;

  /* Settings */
  bool dynamicsIsActive;

  /* Triggers */
  bool cycleTrigger{};

private:
};

#endif

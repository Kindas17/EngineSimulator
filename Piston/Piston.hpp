#ifndef PISTON_HPP
#define PISTON_HPP
#include <numbers>

#include "EngineConfig.hpp"
#include "Gas.hpp"
#include "Orifice.hpp"

constexpr float DEGToRAD(float X) {
  return (2.0 * std::numbers::pi * (X) / 360.f);
}
constexpr float RADToDEG(float X) {
  return (360.0 * (X) / (2 * std::numbers::pi));
}
constexpr float RADSToHZ(float X) {
  return ((X) / (2.f * std::numbers::pi));
}
constexpr float RADSToRPM(float X) {
  return (60.f * RADSToHZ((X)));
}
constexpr float RPMToHz(float X) {
  return X / 60.f;
}
constexpr float RPMToRADS(float X) {
  return 2.f * std::numbers::pi * RPMToHz(X);
}

class Piston {
 public:
  // Piston(CylinderGeometry geometryInfo);
  Piston(EngineConfig cfg);

  /* Specs */
  // CylinderGeometry geometry;
  EngineConfig cfg;
  std::valarray<float> rodFoot{{0.f, 0.f}};

  float throttle{0.f};
  float minThrottle{0.03f};

  float totalFuelConsumption{0.f};

  /* Dynamics */
  bool ignitionOn{false};
  void update(float deltaT);
  void ValveMgm();
  float externalTorque{};
  bool killDynamics{false};

  float combustionAdvance{};
  float kthermal{1.0f};

  /* Generic Methods */
  float getPistonPosition();
  float getCyclePercent();
  float getChamberVolume();
  float getMaxVolume();
  float getEngineVolume();
  float getCompressionRatio();
  float getThetaAngle();
  float getCurrentAngle();
  float getHeadAngle();
  float getTorque();
  float getEngineSpeed();
  std::valarray<float> chamberDisplacement();
  constexpr float getThrottle(float curr);

  void setEngineSpeed(float omega);

  /* Valves */
  float intakeValve{};
  float exhaustValve{};
  float intakeFlow{};
  float exhaustFlow{};
  float leakageFlow{};

  /* Thermodynamics */
  Gas gas;
  Gas intakeManifold;
  Gas exhaustPipe;
  Gas externalAir;
  Orifice intakeValveOrif;
  Orifice exhaustValveOrif;
  Orifice IntakeManifoldOrif;
  Orifice ExhaustPipeOrif;

  bool combustionInProgress;

  /* Settings */
  bool dynamicsIsActive;

  /* Triggers */
  bool cycleTrigger{};

 private:
  std::valarray<float> state = {0.f, 0.f};  // Head angle, omega head
};

#endif

#ifndef PISTON_HPP
#define PISTON_HPP
#include "Linalg.hpp"
#include <stdio.h>
#include "IdealGas.hpp"

#define DEGToRAD(X) (2.0 * M_PI * X / 360.0)
#define RADToDEG(X) (360.0 * X / (2 * M_PI))
#define RADSToHZ(X) (X / (2 * M_PI))
#define RADSToRPM(X) (60.0 * RADSToHZ(X))
#define M3_TO_CC(X) (1000000 * X)
#define MM_TO_M(X) (X / 1000.0)
#define PAToATM(X) (X / 101325.0)
#define KELVToCELS(X) (X - 273)

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

  /* Specs */
  CylinderGeometry geometry;
  vector2_T rodFoot;

  float throttle;
  float minThrottle;

  /* Dynamics */
  bool ignitionOn;
  float omega;
  float headAngle;
  float currentAngle;
  void updatePosition(float deltaT, float setSpeed);
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

  /* Thermodynamics */
  float V_prime;
  IdealGas* gas;

  /* Valves */
  float intakeValve;
  float exhaustValve;
  float intakeFlow;
  float exhaustFlow;
  float leakageFlow;

  /* Settings */
  bool dynamicsIsActive;

  /* Triggers */
  bool cycleTrigger;

private:
};

#endif

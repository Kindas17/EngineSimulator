#include "Logger.hpp"
#include "Piston.hpp"
#include <iostream>
#include <vector>

// Setting up the engine
CylinderGeometry *geom = new CylinderGeometry();
Piston *piston = new Piston(*geom, 0.0f);

void engineReset() {
  geom = new CylinderGeometry();
  piston = new Piston(*geom, 0.0f);
}

extern "C" void setCombustionAdvance(float advance) {
  piston->combustionAdvance = advance;
}

extern "C" void setIntakeExhaustCoefs(float intakeCoef, float exhaustCoef) {
  piston->intakeCoef = intakeCoef;
  piston->exhaustCoef = exhaustCoef;
}

extern "C" void setCombustionSpeed(float speed) {
  piston->combustionSpeed = speed;
}

extern "C" void setThrottle(float throttle) { piston->throttle = throttle; }

extern "C" float GetTorqueAtSpeed(float omega, int iterations) {

  // Setting up the engine
  piston->setEngineSpeed(omega);
  piston->ignitionOn = true;

  // Setting up the simulation
  const float stepsPerCycle = 1000;
  const float simFreq = RADSToHZ(omega) * stepsPerCycle;
  const float deltaT = 1 / simFreq;

  // // Print debug informations
  // std::cout << "Simulation running @ " << simFreq << " Hz" << std::endl;
  // std::cout << "Engine Speed: " << RADSToHZ(omega) << " Hz" << std::endl;
  // std::cout << "Steps per cycle: " << stepsPerCycle << std::endl;

  // Start the simulation
  CycleLogger *torqueLog = new CycleLogger();
  std::vector<float> torqueProfile;
  int reps = 0;

  while (reps < iterations + 1) {
    for (size_t j = 0; j < stepsPerCycle; ++j) {

      piston->update(deltaT);
      torqueLog->addSample(piston->getTorque());

      if (piston->cycleTrigger) {

        // Skip the first trigger as it might be strange
        if (reps != 0) {
          // Save the entire torque profile
          for (size_t i = 0; i < torqueLog->getSize(); ++i) {
            torqueProfile.push_back(torqueLog->getData()[i]);
          }
        }

        torqueLog->trig();
        piston->cycleTrigger = false;
        reps++;
      }
    }
  }

  float outputTorque = 0.0f;
  for (size_t i = 0; i < torqueProfile.size(); ++i) {
    outputTorque += torqueProfile[i];
  }

  engineReset();

  return outputTorque / torqueProfile.size();
}

#include "Logger.hpp"
#include "Piston.hpp"
#include <iostream>
#include <vector>

constexpr float getTimeStep_s(int mult, float frametime) {
  return frametime / (1000.f * mult);
}

extern "C" float GetTorqueAtSpeed(float omega, int iterations) {

  // Setting up the engine
  CylinderGeometry *geom = new CylinderGeometry();
  Piston *piston = new Piston(*geom, omega);
  piston->ignitionOn = true;
  piston->throttle = 1.f;

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

  while (reps < iterations) {
    for (size_t j = 0; j < stepsPerCycle; ++j) {

      piston->update(deltaT);
      torqueLog->addSample(piston->getTorque());

      if (piston->cycleTrigger) {

        // Save the entire torque profile
        for (size_t i = 0; i < torqueLog->getSize(); ++i) {
          torqueProfile.push_back(torqueLog->getData()[i]);
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

  return outputTorque / torqueProfile.size();
}

#include "Logger.hpp"
#include "Piston.hpp"
#include <iostream>

constexpr float getTimeStep_s(int mult, float frametime) {
  return frametime / (1000.f * mult);
}

constexpr int SIMULATION_MULTIPLIER = 200;
constexpr float FRAMETIME = 25.f; /* ms */

float GetTorqueAtSpeed(float omega, int iterations) {
  const float DELTA_T = getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME);
  CylinderGeometry *geom = new CylinderGeometry();
  Piston *piston = new Piston(*geom, omega);
  piston->ignitionOn = true;
  piston->throttle = 1.f;

  float outputTorque = 0.0f;
  int reps = 1;

  CycleLogger *torqueLog = new CycleLogger();

  for (size_t i = 0; i < iterations; ++i) {
    for (size_t j = 0; j < SIMULATION_MULTIPLIER; ++j) {

      const float deltaT = getTimeStep_s(SIMULATION_MULTIPLIER, FRAMETIME);
      const float t =
          0.001f * (i * j * FRAMETIME + j * FRAMETIME / SIMULATION_MULTIPLIER);

      piston->update(deltaT);
      torqueLog->addSample(piston->getTorque());

      if (piston->cycleTrigger) {

        // Save the torque average
        if (torqueLog->getSize() > 0) {
          float sum = 0.f;
          for (size_t i = 0; i < torqueLog->getSize(); ++i) {
            sum += torqueLog->getData()[i];
          }
          outputTorque += sum / torqueLog->getSize();
          // outputTorque = sum / torqueLog->getSize();
          // std::cout << outputTorque << " - " << reps << std::endl;
          reps++;
        }

        torqueLog->trig();
        piston->cycleTrigger = false;
      }
    }
  }

  return outputTorque / reps;
}

#define STEPS 30
float torque_v[STEPS];
float speed_v[STEPS];

int main(int argc, char *argv[]) {
  float speed = 10.f;
  for (size_t i = 0; i < STEPS; ++i) {
    speed += 20;
    speed_v[i] = RADSToRPM(speed * 2);
    torque_v[i] = GetTorqueAtSpeed(speed, 100);
    std::cout << torque_v[i] << " Nm @ " << speed_v[i] << " rpm" << std::endl;
  }
}

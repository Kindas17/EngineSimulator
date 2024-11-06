#ifndef ENGINECONFIG_HPP
#define ENGINECONFIG_HPP

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

constexpr float M3ToCC(float X) {
  return (1000000 * (X));
}

constexpr float MMToM(float X) {
  return ((X) / 1000.0);
}

struct Cylinder {
  float stroke;
  float bore;
  float add_stroke;
};

struct Rod {
  float length;
};

struct Crankshaft {
  float weight;
};

struct Valve {
  float timing;
  float shape;
};

class EngineConfig {
 public:
  Cylinder cylinder;
  Rod rod;
  Crankshaft crankshaft;
  Valve intakeValve;
  Valve exhaustValve;

  float momentOfInertia;

  void evaluate() {
    momentOfInertia = crankshaft.weight * pow(cylinder.stroke * 0.5f, 2);
  }

  bool loadFromFile(const std::string& filename);
};

#endif
#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

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

#endif

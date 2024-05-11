#ifndef IDEALGAS_HPP
#define IDEALGAS_HPP

#define DEFAULT_AMBIENT_PRESSURE (101325.f)

class IdealGas {

public:
  float getP() { return pressure; };
  float getV() { return volume; };
  float getnR() { return nR; };
  float getT() { return temperature; };

  IdealGas(float p, float v, float t);

private:
  float pressure;
  float volume;
  float nR;
  float temperature;
};

#endif

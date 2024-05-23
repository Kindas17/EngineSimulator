#ifndef IDEALGAS_HPP
#define IDEALGAS_HPP

constexpr float DEFAULT_AMBIENT_PRESSURE = 101325.f;
constexpr float DEFAULT_AMBIENT_TEMPERATURE = 300.f;
constexpr float ZERO_CELSIUS_IN_KELVIN = 273.15f;
constexpr float PAToATM(float X) { return ((X) / DEFAULT_AMBIENT_PRESSURE); }
constexpr float KELVToCELS(float X) { return ((X)-ZERO_CELSIUS_IN_KELVIN); }

class IdealGas {

public:
  float getP() { return pressure; };
  float getV() { return volume; };
  float getnR() { return nR; };
  float getT() { return temperature; };

  IdealGas(float p, float v, float t);

  /* Ideal process */
  void AdiabaticCompress(float vprime, float dt);
  float SimpleFlow(float kFlow, float ext_pressure, float ext_temp, float dt);
  void HeatExchange(float kTherm, float ext_temp, float dt);
  void InjectHeat(float qprime, float dt);

protected:
  float pressure;
  float volume;
  float nR;
  float temperature;

  const float alpha = 5.f / 2.f;
};

class Gas : public IdealGas {

public:
  Gas(float p, float v, float t, float o);

  float getOx() { return ox; };
  float SimpleFlow(float kFlow, float ext_pressure, float ext_temp,
                   float ext_ox, float dt);

private:
  float ox; // Oxygenation level [0, 1]
};

#endif

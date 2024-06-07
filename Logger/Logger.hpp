#ifndef LOGGER_HPP_
#define LOGGER_HPP_
#include <vector>

class Logger {
public:
  Logger(std::size_t max_size);
  void addSample(float sample);
  float *getData();
  std::size_t getSize();

private:
  std::size_t max_size;
  std::vector<float> v;
};

class CycleLogger {
private:
  int which;
  std::vector<float> a;
  std::vector<float> b;

public:
  void trig();
  void addSample(float sample);
  float *getData();
  std::size_t getSize();
  std::vector<float> getV();
};

#endif

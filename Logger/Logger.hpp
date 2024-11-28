#ifndef LOGGER_HPP_
#define LOGGER_HPP_
#include <functional>
#include <string>
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
  std::function<float()> sampleFun;

 public:
  CycleLogger(const std::function<float()> &fun) : sampleFun{fun} {
  }

  void trig();
  void addSample();
  float *getData();
  std::size_t getSize();
  std::vector<float> getV();
};

class LoggerMgm {
 private:
  std::unordered_map<std::string, CycleLogger> loggers;

 public:
  float *getLoggerData(std::string name) {
    return loggers.at(name).getData();
  }

  size_t getLoggerSize(std::string name) {
    return loggers.at(name).getSize();
  }

  void addLogger(std::string name, CycleLogger logger) {
    loggers.insert({name, logger});
  }

  void logAll() {
    for (auto &logger : loggers) {
      logger.second.addSample();
    }
  }

  void resetAll() {
    for (auto &logger : loggers) {
      logger.second.trig();
    }
  }
};

#endif

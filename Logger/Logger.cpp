#include "Logger.hpp"

Logger::Logger(int max_size) { this->max_size = max_size; }

void Logger::addSample(float sample) {
  v.push_back(sample);
  if (v.size() > max_size) {
    v.erase(v.begin());
  }
}

float *Logger::getData() { return v.data(); }

int Logger::getSize() { return v.size(); }

void CycleLogger::addSample(float sample) {
  if (which == 0) {
    a.push_back(sample);
  } else {
    b.push_back(sample);
  }
}

float *CycleLogger::getData() {
  if (which == 0) {
    return b.data();
  } else {
    return a.data();
  }
}

int CycleLogger::getSize() {
  if (which == 0) {
    return b.size();
  } else {
    return a.size();
  }
}

void CycleLogger::trig() {
  if (which == 0) {
    which = 1;
    b.clear();
  } else {
    which = 0;
    a.clear();
  }
}

std::vector<float> CycleLogger::getV() { return (which == 0) ? a : b; }

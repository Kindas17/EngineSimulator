#ifndef FRAMERVIS_H_
#define FRAMERVIS_H_

#include <chrono>
#include <vector>

using namespace std::chrono;

class FrameRVis {
 private:
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point end;
  int lastDuration;
  std::vector<float> v;

  void addSample(float sample) {
    v.push_back(sample);
    if (v.size() > 100) {
      v.erase(v.begin());
    }
  }

 public:
  void startClock() {
    start = high_resolution_clock::now();
  }

  void endClock() {
    end = high_resolution_clock::now();
    lastDuration = duration_cast<microseconds>(end - start).count();
    addSample((1000000.f / lastDuration));
  }

  float getLast() {
    return lastDuration / 1000.f;
  }

  size_t getSize() {
    return v.size();
  }

  float *getData() {
    return v.data();
  }

  float getFramerate() {
    if (!v.empty()) {
      return v.back();
    }
    return 0.f;
  }
};

#endif

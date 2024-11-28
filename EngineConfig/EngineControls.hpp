#ifndef ENGINECONTROLS_HPP
#define ENGINECONTROLS_HPP

#include <mutex>

struct EngineControls {
  float externalTorque;
  float throttle;
  bool ignitionOn;
};

struct EngineState {
  float engineSpeed;
};

class EngineControlsMgm {
 private:
  EngineControls ctrls;
  EngineState state;
  std::mutex mtx;

 public:
  EngineControlsMgm() : ctrls{}, state{} {};

  void setState(EngineState st) {
    std::lock_guard<std::mutex> lock(mtx);
    state = st;
  };
  EngineState getState() {
    std::lock_guard<std::mutex> lock(mtx);
    return state;
  };

  void setControls(EngineControls inCtrls) {
    std::lock_guard<std::mutex> lock(mtx);
    ctrls = inCtrls;
  };
  EngineControls getControls() {
    std::lock_guard<std::mutex> lock(mtx);
    return ctrls;
  };
};

#endif
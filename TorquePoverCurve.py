from ctypes import *
import numpy as np
import matplotlib.pyplot as plt


def DEGToRAD(deg):
    return 2 * np.pi * deg / 360


def engineSetup(lib):
    lib.setCombustionAdvance(c_float(DEGToRAD(15)))
    lib.setIntakeExhaustCoefs(c_float(0.00008), c_float(0.00008))
    lib.setCombustionSpeed(c_float(15000))
    lib.setThrottle(c_float(1))


STEPS = 40

lib = cdll.LoadLibrary("build/libtorqueCurve.so")
lib.GetTorqueAtSpeed.restype = c_float

speed = 50
speed_log, torque_log = [], []
for i in range(STEPS):
    # Setup the engine parameters
    engineSetup(lib)
    speed += 24
    torque = lib.GetTorqueAtSpeed(c_float(speed), c_int(1))
    speed_log.append(60 * 0.5 * speed / np.pi)
    torque_log.append(torque)
    print("{:.2f} Nm @ {:.0f} rpm".format(torque, 60 * 0.5 * speed / np.pi))
    if torque < 0:  # Force stop if the engine is not producing positive torque
        break

fig, axs = plt.subplots(2, sharex=True)

axs[0].plot(speed_log, torque_log)
axs[0].set_xlabel("Engine Speed [RPM]")
axs[0].set_ylabel("Torque [Nm]")
axs[0].grid()

axs[1].plot(
    speed_log,
    [0.001 * t * (rpm / 60) * 2 * np.pi for t, rpm in zip(torque_log, speed_log)],
)
axs[1].set_xlabel("Engine Speed [RPM]")
axs[1].set_ylabel("Power [kW]")
axs[1].grid()
plt.show()

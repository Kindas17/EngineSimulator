from ctypes import *
import numpy as np
import matplotlib.pyplot as plt

STEPS = 40

lib = cdll.LoadLibrary("build/libtorqueCurve.so")
lib.GetTorqueAtSpeed.restype = c_float

speed = 50
speed_log, torque_log = [], []
for i in range(STEPS):
    speed += 12
    torque = lib.GetTorqueAtSpeed(c_float(speed), c_int(10))
    speed_log.append(60 * speed / np.pi)
    torque_log.append(torque)
    print("{:.2f} Nm @ {:.0f} rpm".format(torque, 60 * speed / np.pi))

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

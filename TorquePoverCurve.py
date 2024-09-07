from ctypes import CDLL, c_float
import numpy as np
import matplotlib.pyplot as plt

SIZE = 30

lib = CDLL("build/libtorqueCurve.so")
lib.main()

speed_v = np.array(list((c_float * SIZE).in_dll(lib, "speed_v")))
torque_v = np.array(list((c_float * SIZE).in_dll(lib, "torque_v")))

fig, axs = plt.subplots(2, sharex=True)

axs[0].plot(speed_v, torque_v)
axs[0].set_xlabel("Engine Speed [RPM]")
axs[0].set_ylabel("Torque [Nm]")
axs[0].grid()

axs[1].plot(
    speed_v, [0.001 * t * (rpm / 60) * 2 * np.pi for t, rpm in zip(torque_v, speed_v)]
)
axs[1].set_xlabel("Engine Speed [RPM]")
axs[1].set_ylabel("Power [kW]")
axs[1].grid()
plt.show()

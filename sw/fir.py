import numpy as np
from scipy.signal import firwin, freqz
import matplotlib.pyplot as plt

fs = 125e6
passband = 26e6
stopband = 28e6
cutoff = (passband + stopband) / 2

num_taps = 101

coefficients = firwin(num_taps, cutoff, fs=fs, pass_zero=True)

print("Filter Coefficients (comma delimited for vivado FIR compiler):")
print(",".join(map(str, coefficients)))

w, h = freqz(coefficients, worN=8000, fs=fs)
plt.plot(w, 20 * np.log10(abs(h)), 'b')
plt.title("Low Pass Filter Frequency Response")
plt.xlabel("Frequency (Hz)")
plt.ylabel("Gain (dB)")
plt.grid(True)
plt.show()


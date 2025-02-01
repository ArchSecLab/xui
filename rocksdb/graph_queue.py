import numpy as np
import matplotlib.pyplot as plt

filename = 'queue'
with open(filename, 'r') as f:
    x = f.readlines()
    x = np.array([float(s.split('\n')[0]) for s in x])
    indices = np.arange(len(x))
    mods = np.fmod(indices,4)
    ind_mod = indices[mods]
    periods = x[indices[ind_mod == 0]]
    rate_high = x[indices[ind_mod == 1]]
    rate = x[indices[ind_mod == 2]]
    rate_low = x[indices[ind_mod == 3]]


plt.plot(periods,rate_high,label = "High Rate")
plt.plot(periods,rate,label = "Rate")
plt.plot(periods,rate_low,label = "Low Rate")
plt.title("Queue")

plt.xlabel("Send Period")
plt.ylabel("Recv Rate")
plt.show()
import tikzplotlib
tikzplotlib.save("queue.tex")

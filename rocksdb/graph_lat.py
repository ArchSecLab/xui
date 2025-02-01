import numpy as np
import matplotlib.pyplot as plt

filename = 'flushlat'
with open(filename, 'r') as f:
    x = f.readlines()
    y = [float(s.split('\n')[0]) for s in x]
    x = [i+1 for i in range(len(x))]

x = np.array(x)
y = np.array(y)/500
plt.plot(x,y,label = "Flush Latency")
filename = 'drainlat'
with open(filename, 'r') as f:
    x = f.readlines()
    y = [float(s.split('\n')[0]) for s in x]
    x = [i+1 for i in range(len(x))]

x = np.array(x)
y = np.array(y)/500
plt.yscale("log")
plt.plot(x,y,label = "Drain Latency")
plt.title("End To End Latency")

plt.xlabel("Buffer Size (2^14)")
plt.ylabel("ln(Latency in Cycles)")
plt.show()
import tikzplotlib
tikzplotlib.save("lat.tex")

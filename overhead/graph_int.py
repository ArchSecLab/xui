import numpy as np
import matplotlib.pyplot as plt

filename = 'flushint'
with open(filename, 'r') as f:
    x = f.readlines()
    y = [float(s.split('\n')[0]) for s in x]
    x = [i+1 for i in range(len(x))]

x = np.array(x)
y = np.array(y)
plt.plot(x,y,label = "#Interrupts with Flushing")
filename = 'drainint'
with open(filename, 'r') as f:
    x = f.readlines()
    y = [float(s.split('\n')[0]) for s in x]
    x = [i+1 for i in range(len(x))]

x = np.array(x)
y = np.array(y)
plt.plot(x,y,label = "#Interrupts with Draining")
plt.title("#Interrupts")

plt.xlabel("Buffer Size")
plt.ylabel("Recieved Interrupt Count")
import tikzplotlib
tikzplotlib.save("int.tex")

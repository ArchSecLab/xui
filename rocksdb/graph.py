import numpy as np
import matplotlib.pyplot as plt
import re
def find_in_file(search,f):
	for line in f.readlines():
		if search in line:
			return line
	return "no 0"
def get_column(line, c):
	return line.split()[c]
int_types = ["NoInterrupt","Drain","Flush","Track"]

for int_t in int_types:
	x = []
	y = []
	for i in range(32):
		with open("stats/stat"+int_t+"Size"+str(i)+".txt",'r') as f:
			l = find_in_file('board.processor.switch1.core.numROICycles',f)
			y.append(float(get_column(l,1)))
		with open("stats/stat"+int_t+"Size"+str(i)+".txt",'r') as f:
			l = find_in_file('board.processor.switch1.core.numUserInterruptsDelivered',f)
			x.append(float(get_column(l,1)))
	x = np.array(x)
	y = np.array(y)
	plt.plot(x,y)
		
plt.title("Cycles wrt Interrupt Count")

plt.xlabel("Interrupt Count")
plt.ylabel("Cycle")
plt.show()
import tikzplotlib
tikzplotlib.save("cyclecount.tex")
"""
x = np.array(x)
y = np.array(y)
plt.plot(x,y,label = "Flush IPC")
filename = 'drainipc'
with open(filename, 'r') as f:
    x = f.readlines()
    y = [float(s.split('\n')[0]) for s in x]
    x = [i+1 for i in range(len(x))]

x = np.array(x)
y = np.array(y)
plt.plot(x,y,label = "Drain IPC")
filename = 'vanillaipc'
with open(filename, 'r') as f:
    x = f.readlines()
    y = [float(s.split('\n')[0]) for s in x]
    x = [i+1 for i in range(len(x))]

x = np.array(x)
y = np.array(y)
plt.plot(x,y,label = "Vanilla IPC")
plt.title("Cycles wrt Buffer Size")

plt.xlabel("Buffer Size")
plt.ylabel("Cycle")
plt.show()
import tikzplotlib
tikzplotlib.save("ipc.tex")
"""

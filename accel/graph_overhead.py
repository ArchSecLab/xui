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
int_types = ["Drain","Flush","Track"]
possibles = [5000, 1000, 500, 200, 100, 50, 20, 10, 5, 4, 3, 2, 1]
with open("boards/bTrackmatmul0",'r') as f:
	l = find_in_file('Execution',f)
	y_orig= float(get_column(l,1))
print(y_orig)
for int_t in int_types:
	x = []
	y = []
	for ind,i in enumerate(possibles):
		try:
			with open("boards/b"+int_t+"matmul"+str(i),'r') as f:
				l = find_in_file('Execution',f)
				if(l == "no 0"):
					continue
				y.append((float(get_column(l,1))/y_orig -1)*100)
				x.append(i)
		except:
			pass
	x = np.array(x)
	y = np.array(y)
	plt.plot(x,y)
		
plt.title("Interrupt Overhead matmul")

plt.xlabel("Interrupt Period(us)")
plt.ylabel("Execution Time Overhead %")
plt.show()
import tikzplotlib
tikzplotlib.save("overhead.tex")
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

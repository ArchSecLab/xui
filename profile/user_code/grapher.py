from matplotlib import pyplot as plt
import statistics

file = open('log', 'r')
lines = file.readlines()
file.close()

current = 0

t0t1 = []
t1t2 = []
t2t3 = []
t3t4 = []
t4t5 = []
t5t6 = []
t6t7 = []
total = []

for line in lines:
    if line == '\n':
        continue
    if line == 't0t1:\n':
        continue
    if line == 't1t2:\n':
        current = 1
        continue
    if line == 't2t3:\n':
        current = 2
        continue
    if line == 't3t4:\n':
        current = 3
        continue
    if line == 't4t5:\n':
        current = 4
        continue
    if line == 't5t6:\n':
        current = 5
        continue
    if line == 't6t7:\n':
        current = 6
        continue
    if line == 'total:\n':
        current = 7
        continue
    if current == 0:
        t0t1.append(float(line))
    if current == 1:
        t1t2.append(float(line))
    if current == 2:
        t2t3.append(float(line))
    if current == 3:
        t3t4.append(float(line))
    if current == 4:
        t4t5.append(float(line))
    if current == 5:
        t5t6.append(float(line))
    if current == 6:
        t6t7.append(float(line))
    if current == 7:
        total.append(float(line))
# Calculate the number of bins dynamically based on the range of values
num_bins = int(int(max(total) - min(total))/100) + 1
# Plot histograms for each category
plt.hist(t0t1, bins=num_bins, label='t0t1')
plt.legend()
plt.savefig('t0t1.png')
plt.clf()
plt.hist(t1t2, bins=num_bins, label='t1t2')
plt.legend()
plt.savefig('t1t2.png')
plt.clf()
plt.hist(t2t3, bins=num_bins, label='t2t3')
plt.legend()
plt.savefig('t2t3.png')
plt.clf()
plt.hist(t3t4, bins=num_bins, label='t3t4')
plt.legend()
plt.savefig('t3t4.png')
plt.clf()
plt.hist(t4t5, bins=num_bins, label='t4t5')
plt.legend()
plt.savefig('t4t5.png')
plt.clf()
plt.hist(t5t6, bins=num_bins, label='t5t6')
plt.legend()
plt.savefig('t5t6.png')
plt.clf()
plt.hist(t6t7, bins=num_bins, label='t6t7')
plt.legend()
plt.savefig('t6t7.png')
plt.clf()
plt.hist(total, bins=2, label='total')
plt.legend()
plt.savefig('graph.png')
# Calculate the median of each array
t0t1_median = statistics.median(t0t1)
t1t2_median = statistics.median(t1t2)
t2t3_median = statistics.median(t2t3)
t3t4_median = statistics.median(t3t4)
t4t5_median = statistics.median(t4t5)
t5t6_median = statistics.median(t5t6)
t6t7_median = statistics.median(t6t7)
total_median = statistics.median(total)

# Print the medians
print("Median of t0t1:", t0t1_median)
print("Median of t1t2:", t1t2_median)
print("Median of t2t3:", t2t3_median)
print("Median of t3t4:", t3t4_median)
print("Median of t4t5:", t4t5_median)
print("Median of t5t6:", t5t6_median)
print("Median of t6t7:", t6t7_median)
print("Median of total:", total_median)
# Calculate the mode of each array
t0t1_mode = statistics.mode(t0t1)
t1t2_mode = statistics.mode(t1t2)
t2t3_mode = statistics.mode(t2t3)
t3t4_mode = statistics.mode(t3t4)
t4t5_mode = statistics.mode(t4t5)
t5t6_mode = statistics.mode(t5t6)
t6t7_mode = statistics.mode(t6t7)
total_mode = statistics.mode(total)

# Print the modes
print("Mode of t0t1:", t0t1_mode)
print("Mode of t1t2:", t1t2_mode)
print("Mode of t2t3:", t2t3_mode)
print("Mode of t3t4:", t3t4_mode)
print("Mode of t4t5:", t4t5_mode)
print("Mode of t5t6:", t5t6_mode)
print("Mode of t6t7:", t6t7_mode)
print("Mode of total:", total_mode)
#plt.hist(t0t1, bins=2, label='t0t1')
#plt.legend()
#plt.savefig('t0t1.png')
#plt.clf()
#plt.hist(t1t2, bins=2, label='t1t2')
#plt.legend()
#plt.savefig('t1t2.png')
#plt.clf()
#plt.hist(t2t3, bins=2, label='t2t3')
#plt.legend()
#plt.savefig('t2t3.png')
#plt.clf()
#plt.hist(t3t4, bins=2, label='t3t4')
#plt.legend()
#plt.savefig('t3t4.png')
#plt.clf()
#plt.hist(t4t5, bins=2, label='t4t5')
#plt.legend()
#plt.savefig('t4t5.png')
#plt.clf()
#plt.hist(total, bins=2, label='total')
#plt.legend()
#plt.savefig('graph.png')
"""plt.plot(t0t1, label='t0t1')
plt.legend()
plt.savefig('t0t1.png')
#get rid of the plot
plt.clf()
plt.plot(t1t2, label='t1t2')
plt.legend()
plt.savefig('t1t2.png')
plt.clf()
plt.plot(t2t3, label='t2t3')
plt.legend()
plt.savefig('t2t3.png')
plt.clf()
plt.plot(t3t4, label='t3t4')
plt.legend()
plt.savefig('t3t4.png')
plt.clf()
plt.plot(t4t5, label='t4t5')
plt.legend()
plt.savefig('t4t5.png')
plt.clf()
plt.plot(total, label='total')
plt.legend()
plt.savefig('graph.png')"""

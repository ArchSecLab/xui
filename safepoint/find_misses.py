import os
import re

import matplotlib.pyplot as plt

# Directory containing the stat files
stat_dir = 'stats'

# Regex pattern to match the required files
file_pattern = re.compile(r'statschase\w+_\d+_\d+')

# Regex pattern to extract the miss value
miss_pattern = re.compile(r'l1dcaches1\.overallMissRate::total\s+(\d+\.\d+)')

# Dictionary to store the miss values
misses = {}

# Iterate over the files in the stat directory
for filename in os.listdir(stat_dir):
    if file_pattern.match(filename):
        with open(os.path.join(stat_dir, filename), 'r') as file:
            content = file.read()
            match = miss_pattern.search(content)
            if match:
                misses[filename] = float(match.group(1))

# Sort the misses by filename
# Separate misses into Flush and Drain categories
flush_misses = {}
drain_misses = {}

for filename, miss in misses.items():
    if 'Flush' in filename:
        flush_misses[filename] = miss
    elif 'Drain' in filename:
        drain_misses[filename] = miss

# Sort the misses by filename
sorted_flush_misses = dict(sorted(flush_misses.items()))
sorted_drain_misses = dict(sorted(drain_misses.items()))

# Plot the misses
plt.figure(figsize=(10, 5))
plt.plot(list(sorted_flush_misses.keys()), list(sorted_flush_misses.values()), marker='o', label='Flush', color='blue')
plt.plot(list(sorted_drain_misses.keys()), list(sorted_drain_misses.values()), marker='o', label='Drain', color='red')
plt.xlabel('File')
plt.ylabel('miss (mean)')
plt.title('miss for Flush and Drain Strategies')
plt.xticks(rotation=90)
plt.legend()
plt.tight_layout()
plt.savefig('miss_plot.png')
plt.show()

# Plot the misses
# plt.figure(figsize=(10, 5))
# plt.plot(list(sorted_misses.keys()), list(sorted_misses.values()), marker='o')
# plt.xlabel('File')
# plt.ylabel('End-to-End miss (mean)')
# plt.title('End-to-End miss for Different Strategies and Intervals')
# plt.xticks(rotation=90)
# plt.tight_layout()
# plt.savefig('miss_plot.png')
# plt.show()
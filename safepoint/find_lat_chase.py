import os
import re

import matplotlib.pyplot as plt

# Directory containing the stat files
stat_dir = 'stats'

# Regex pattern to match the required files
file_pattern = re.compile(r'statschase\w+_\d+_\d+')

# Regex pattern to extract the latency value
latency_pattern = re.compile(r'switch1\.core\.endToEndSendUipiLatency::mean\s+(\d+\.\d+)')

# Dictionary to store the latency values
latencies = {}

# Iterate over the files in the stat directory
for filename in os.listdir(stat_dir):
    if file_pattern.match(filename) and "10000" in filename and "Drain" in filename:
        with open(os.path.join(stat_dir, filename), 'r') as file:
            content = file.read()
            match = latency_pattern.search(content)
            if match:
                latencies[filename] = float(match.group(1))

# Sort the latencies by filename
# Separate latencies into Flush and Drain categories
flush_latencies = {}
drain_latencies = {}

for filename, latency in latencies.items():
    if 'Flush' in filename:
        flush_latencies[filename] = latency
    elif 'Drain' in filename:
        drain_latencies[filename] = latency

# Sort the latencies by filename
sorted_flush_latencies = dict(sorted(flush_latencies.items()))
sorted_drain_latencies = dict(sorted(drain_latencies.items()))

# Plot the latencies
plt.figure(figsize=(10, 5))
plt.plot(list(sorted_flush_latencies.keys()), list(sorted_flush_latencies.values()), marker='o', label='Flush', color='blue')
plt.plot(list(sorted_drain_latencies.keys()), list(sorted_drain_latencies.values()), marker='o', label='Drain', color='red')
plt.xlabel('File')
plt.ylabel('End-to-End Latency (mean)')
plt.title('End-to-End Latency for Flush and Drain Strategies')
plt.xticks(rotation=90)
plt.legend()
plt.tight_layout()
plt.savefig('latency_plot.png')
plt.show()

# Plot the latencies
# plt.figure(figsize=(10, 5))
# plt.plot(list(sorted_latencies.keys()), list(sorted_latencies.values()), marker='o')
# plt.xlabel('File')
# plt.ylabel('End-to-End Latency (mean)')
# plt.title('End-to-End Latency for Different Strategies and Intervals')
# plt.xticks(rotation=90)
# plt.tight_layout()
# plt.savefig('latency_plot.png')
# plt.show()
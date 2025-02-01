import os
import re
import sys
import statistics
import matplotlib.pyplot as plt
from matplotlib import ticker

# Function to extract values from a single file
def extract_values(file_path):
    result, time, preemption, sent = None, None, None, None

    # Regular expressions to match the desired lines
    result_pattern = re.compile(r"Result:\s*(-?\d+)")
    time_pattern = re.compile(r"Time:\s*(\d+)")
    preemption_pattern = re.compile(r"Preemption:\s*(\d+)")
    sent_pattern = re.compile(r"Sent:\s*(\d+)")

    # Read the file line by line and extract values
    with open(file_path, 'r') as file:
        for line in file:
            if result_match := result_pattern.search(line):
                result = int(result_match.group(1))
            if time_match := time_pattern.search(line):
                time = int(time_match.group(1))
            if preemption_match := preemption_pattern.search(line):
                preemption = int(preemption_match.group(1))
            if sent_match := sent_pattern.search(line):
                sent = int(sent_match.group(1))

    return result, time, preemption, sent

def calculate_median_slowdown(folder_path, baseline):
    metrics = []

    # Loop through all files in the folder
    for filename in os.listdir(folder_path):
        file_path = os.path.join(folder_path, filename)
        file_path = os.path.join(file_path, 'board.pc.com_1.device')
        
        # Skip directories, focus on files only
        if os.path.isfile(file_path):
            result, time, preemption, sent = extract_values(file_path)

            # Ensure preemption is not zero to avoid division by zero
            if time != None:
                # metric = (time - baseline) / preemption
                metric = (time - baseline) / baseline
                # print(f'{folder_path}: {metric} = ({time} - {baseline}) / {baseline}')
                if metric < 1: # drop large noise
                    metrics.append(metric)

    # Return the median of the collected metrics
    if metrics:
        # print(f"========= Median: {statistics.median(metrics)}")
        return statistics.median(metrics)
    else:
        return None  # Return None if no metrics are calculated

def calculate_median_time(folder_path):
    times = []

    # Loop through all files in the folder
    for filename in os.listdir(folder_path):
        file_path = os.path.join(folder_path, filename)
        file_path = os.path.join(file_path, 'board.pc.com_1.device')

        # Skip directories, focus on files only
        if os.path.isfile(file_path):
            result, time, preemption, sent = extract_values(file_path)

            # Collect the time value
            if time is not None:
                times.append(time)

    # print("times:", times)
    # Return the median of the collected time values
    if times:
        return statistics.median(times)
    else:
        return None  # Return None if no times are found
    

maxq = 100000000
modes = ['concord', 'uintr', 'safepoint']
names = ['Polling', 'UIPI', 'xUI: Hardware Safepoints']
quanta = [200, 100, 50, 20, 10, 5, 2]
colors = ['#ffab59', '#7dabd1',  '#ed6a6b']



def plot2(data1, data2, save_dir):
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(6, 2.3))

    idx = 0
    for mode in data1.keys():
        ax1.plot(quanta, data1[mode], marker='o', fillstyle='none', linewidth=2, color=colors[idx], label=names[idx])
        idx += 1
    
    # ax1.set_xlabel(r'Preemption Quantum ($\mu$s)', fontsize=13)
    ax1.set_ylabel('Slowdown', fontsize=13)
    ax1.invert_xaxis()
    ax1.set_xscale("log")
    quanta_shown = [200, 100, 50, 20, 10, 5, 2]
    ax1.set_ylim(bottom=0, top=0.15)
    ax1.set_yticks([0, 0.05, 0.1, 0.15])
    ax1.set_xticks(quanta_shown, quanta_shown)
    ax1.tick_params(axis='x', labelsize=10)
    ax1.yaxis.set_major_formatter(ticker.PercentFormatter(1, decimals=0))

    ax1.text(0.06, 0.93, "matmul", transform=ax1.transAxes, 
         fontsize=13, verticalalignment='top', horizontalalignment='left')

    idx = 0
    for mode in data2.keys():
        ax2.plot(quanta, data2[mode], marker='o', fillstyle='none', linewidth=2, color=colors[idx], label=names[idx])
        idx += 1
    
    ax2.invert_xaxis()
    ax2.set_xscale("log")
    quanta_shown = [200, 100, 50, 20, 10, 5, 2]
    ax2.set_ylim(bottom=0, top=0.15)
    ax2.set_yticks([0, 0.05, 0.1, 0.15])
    ax2.set_xticks(quanta_shown, quanta_shown)
    ax2.tick_params(axis='x', labelsize=10)
    # ax2.set_title("linapck")
    ax2.yaxis.set_major_formatter(ticker.PercentFormatter(1, decimals=0))

    ax2.text(1.32, 0.93, "base64", transform=ax1.transAxes, 
         fontsize=13, verticalalignment='top', horizontalalignment='left')

    fig.text(0.5, 0.04, r'Preemption Quantum ($\mu$s)', ha='center', va='center', fontsize=13)

    handles, labels = ax1.get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper center", ncol=3, frameon=False, fontsize=13, handlelength=1.3, columnspacing=1.5, handletextpad=0.5)

    plt.tight_layout()
    plt.subplots_adjust(top=0.8, bottom=0.23, left=0.12, right=0.97)
    plt.savefig(f'{save_dir}/figure6.pdf')
    plt.show()


MAIN_DIR = os.path.dirname(os.path.abspath(__file__))
RESULT_DIR = f'{MAIN_DIR}/result/safepoint'

resultpath = f'{RESULT_DIR}/matmul/100'
Baseline = calculate_median_time(os.path.join(os.path.join(resultpath, 'base'), str(maxq)))
UBaseline = calculate_median_time(os.path.join(os.path.join(resultpath, 'uintr'), str(maxq)))
data1 = {}

for mode in modes:
    tmp = []
    for q in quanta:
        baseline = UBaseline if mode == 'uintr' else Baseline
        slowdown = calculate_median_slowdown(os.path.join(os.path.join(resultpath, mode), str(q)), baseline)
        tmp.append(slowdown)
    data1[mode] = tmp


resultpath = f'{RESULT_DIR}/base64/1'
Baseline = calculate_median_time(os.path.join(os.path.join(resultpath, 'base'), str(maxq)))
SBaseline = calculate_median_time(os.path.join(os.path.join(resultpath, 'safepoint'), str(maxq)))
data2 = {}

for mode in modes:
    tmp = []
    for q in quanta:
        baseline = SBaseline if mode == 'safepoint' else Baseline
        slowdown = calculate_median_slowdown(os.path.join(os.path.join(resultpath, mode), str(q)), baseline)
        tmp.append(slowdown)
    data2[mode] = tmp

plot2(data1, data2, RESULT_DIR)
import sys
import re
import os
import subprocess
import pandas as pd
import matplotlib.pyplot as plt
import math

def extract_latency(file_path, req_type, latency):
    """Extracts latency values from a given file using grep."""
    latency_map = {
        "Median": 7,
        "90th": 8,
        "95th": 9,
        "99th": 10,
        "99.9th": 11,
        "99.99th": 12
    }
    
    if latency not in latency_map:
        raise ValueError(f"Invalid latency type: {latency}")
    
    try:
        # Use grep to find the line starting with req_type
        result = subprocess.run(["grep", f"^{req_type}", file_path], capture_output=True, text=True, check=True)
        lines = result.stdout.splitlines()
        
        if lines:
            parts = lines[0].split(',')
            float_value = float(parts[latency_map[latency]])
            if math.isinf(float_value):
                return 1e5
            return float_value
    except subprocess.CalledProcessError:
        # print(f"No matching line found for {req_type}")
        return 1e5
    except (ValueError, IndexError) as e:
        # print(f"Error processing latency data: {e}")
        return 1e5
    
    return 1e5


mpps1 = [0.02, 0.04, 0.06, 0.08, 0.13, 0.14, 0.15, 0.16, 0.17, 0.18]
mpps2 = [0.02, 0.06, 0.08, 0.10, 0.12, 0.13, 0.14, 0.145, 0.15, 0.155, 0.16, 0.165, 0.18]
system = ['base', 'uintr', 'fast']
systemName = ['Non-preemptive', 'UIPI SW Timer',  'UI KB Timer + Tracking']
quantum = ['100000000us', '5us', '5us']

MAIN_DIR = os.path.dirname(os.path.abspath(__file__))
RESULT_DIR = f'{MAIN_DIR}/result/rocksdb'

def collect(req_type, lat_type):
    data = {}
    for idx, s in enumerate(system):
        name = systemName[idx]
        q = quantum[idx]
        data[name] = []
        mpps = mpps1 if name == 'Non-preemptive' else mpps2
        for m in mpps:
            resultpath = '{}/{}/{}/{:.3f}'.format(RESULT_DIR, s, q, m)
            lats = []
            med = 1e5
            try:
                for trypath in os.listdir(resultpath):
                    if os.path.isdir(os.path.join(resultpath, trypath)) and trypath.startswith("try_"):
                        filename = '{}/{}/board.pc.com_1.device'.format(resultpath, trypath)
                        lat = extract_latency(filename, req_type, lat_type)
                        if lat is not None:
                            lats.append(lat)
                lats = sorted(lats)
                if len(lats) == 0:
                    med = 1e5
                else:
                    med = lats[(len(lats)-1)//2]
            except FileNotFoundError:
                pass
            print(f"{req_type} ({s}, {q} us, {m} mpps): {med}")
            data[name].append(med)    
    return data


color = ['#ffab59', '#7dabd1',  '#ed6a6b']
marker = ['o', 'x', 's']

def plot_alltypes(data1, data2):
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(6, 2.5))

    req_type = 'Get'
    for idx, name in enumerate(systemName):
        mpps = mpps1 if name == 'Non-preemptive' else mpps2
        req_ratio = 0.995 if req_type == 'Get' else 0.005
        X_filtered = [x*req_ratio for x, y in zip(mpps, data1[name]) if y is not None]
        Y_filtered = [y for y in data1[name] if y is not None]
        ax1.plot(X_filtered, Y_filtered, linewidth=1.8, marker=marker[idx], markersize=6, c=color[idx], markerfacecolor='none', label=name)
    
    ax1.set_xlim(left=0, right=0.16)
    ax1.set_ylim(bottom=0, top=1000)
    ax1.set_xticks([0.0, 0.04, 0.08, 0.12, 0.16], [0, 40, 80, 120, 160])
    ax1.set_yticks([0, 500, 1000], [0, 0.5, 1])
    ax1.set_xlabel('KRPS', fontsize=13)
    ax1.set_ylabel(r'99% Latency (ms)', fontsize=13)
    ax1.text(0.06, 0.93, "GET", transform=ax1.transAxes, 
         fontsize=13, verticalalignment='top', horizontalalignment='left')

    req_type = 'Scan'
    for idx, name in enumerate(systemName):
        mpps = mpps1 if name == 'Non-preemptive' else mpps2
        req_ratio = 0.995 if req_type == 'Get' else 0.005
        X_filtered = [x*req_ratio for x, y in zip(mpps, data2[name]) if y is not None]
        Y_filtered = [y for y in data2[name] if y is not None]
        ax2.plot(X_filtered, Y_filtered, linewidth=1.8, marker=marker[idx], markersize=6, c=color[idx], markerfacecolor='none', label=name)
    
    ax2.set_xlim(left=0, right=0.0009)
    ax2.set_ylim(bottom=0, top=30000)
    ax2.set_xticks([0, 0.0003, 0.0006, 0.0009], [0, 0.3, 0.6, 0.9])        
    ax2.set_yticks([0, 10000, 20000, 30000], [0, 10, 20, 30])        
    ax2.set_xlabel('KRPS', fontsize=13)
    ax2.text(1.35, 0.93, "SCAN", transform=ax1.transAxes, 
         fontsize=13, verticalalignment='top', horizontalalignment='left')

    handles, labels = ax1.get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper center", ncol=3, frameon=False, fontsize=12, handlelength=1.3, columnspacing=0.8, handletextpad=0.3)

    plt.tight_layout()
    plt.subplots_adjust(top=0.77, bottom=0.2)
    plt.savefig(f'{RESULT_DIR}/figure7.pdf')
    plt.show()


data1 = collect('Get', '99th')
data2 = collect('Scan', '99th')
plot_alltypes(data1, data2)
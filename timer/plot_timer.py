import sys
import os
import re
import pandas as pd
import matplotlib.pyplot as plt
import statistics
import brewer2mpl
import matplotlib.ticker as mticker


def get_data(filename):
    flag = False
    with open(filename, "r") as file:
        for line in file:
            columns = re.split(r'\s+', line)
            if 'Throughput:' in columns:
                return int(columns[1])
    return None

MAIN_DIR = os.path.dirname(os.path.abspath(__file__))
RESULT_DIR = f'{MAIN_DIR}/result'

trial = 9
modes = ['sleep', 'itimer']
Tname = [r'100 $\mu$s', r'50 $\mu$s', r'20 $\mu$s', r'10 $\mu$s', r'5 $\mu$s', r'3 $\mu$s']
T = [100, 50, 20, 10, 5, 3]
data = {}
Ncore = 19

def collect_baseline():
    baseline_dir = f'{RESULT_DIR}/base/100/1'
    thrs = []
    for i in range(1, trial+1):
        filename = f'{baseline_dir}/{i}'
        thr = get_data(filename)
        if thr != None:
            thrs.append(thr)
    if len(thrs) == 0:
        print(f"No valid baseline data; Please check folder {baseline_dir} and run `run_timer.sh` again.")
        exit(-1)
    else:
        median = statistics.median(thrs)
    return median

baseline = collect_baseline()

def collect():
    for mode in modes:
        data[mode] = {}
        for idx, t in enumerate(T):
            if mode == 'sleep' and t < 50:
                continue
            tname = Tname[idx]
            data[mode][tname] = []
            ncore = 8 if tname == r'3 $\mu$s' else Ncore 
            for core in range(1, ncore+1):
                thrs = []
                for i in range(1, trial+1):
                    filename = '{}/{}/{}/{}/{}'.format(RESULT_DIR, mode, t, core, i)
                    thr = get_data(filename)
                    if thr != None:
                        thrs.append(thr)
                percent = None
                if len(thrs) == 0:
                    percent = 1
                    print("{} us, {} cores, no data to use!".format(t, core))
                else:
                    median = statistics.median(thrs)
                    percent = 1 - median/baseline
                data[mode][tname].append(percent)


colors = ['#377eb8', '#ff7f00', '#4daf4a',
                  '#f781bf', '#a65628', '#984ea3',
                  '#999999', '#e41a1c', '#dede00']
markers = {'sleep': 'x', 'itimer': 'o'}
markers_list = ['x', 'o']

def plot():
    plt.figure(figsize=(5, 2.4))
    Xcore = [i for i in range(1, Ncore+1)]

    for mode in modes:
        for idx, tname in enumerate(list(data[mode].keys())):
            print('{}: {}'.format(mode, tname))
            Ydata = data[mode][tname]
            if tname == r'5 $\mu$s':
                Xcore = Xcore[0:12]
                Ydata = Ydata[0:12]
            elif tname == r'3 $\mu$s':
                Xcore = Xcore[0:3]
                Ydata = Ydata[0:3]
            print(tname, Ydata)    
            plt.plot(Xcore, Ydata, linewidth=1.5, marker=markers[mode], markersize=5, c=colors[idx], markerfacecolor='none', label='{}'.format(tname))

    plt.axhline(y=1, color='gray', linestyle='dashed')
    legend1_labels = Tname
    legend1_lines = [plt.Line2D([0], [0], color=colors[i], lw=2) for i in range(len(Tname))]
    legend1 = plt.legend(legend1_lines[::-1], legend1_labels[::-1], fontsize=11, ncol=6, frameon=False, handlelength=1, columnspacing=1, handletextpad=0.3,  loc='lower center', bbox_to_anchor=(0.45, 1.1),)

    legend2_labels = ['nanosleep', 'itimer']
    legend2_lines = [
        plt.Line2D([0], [0], color='black', marker=markers_list[0], markerfacecolor='none',  markersize=4, linestyle='None'),
        plt.Line2D([0], [0], color='black', marker=markers_list[1], markerfacecolor='none',  markersize=4, linestyle='None')
    ]
    legend2 = plt.legend(legend2_lines, legend2_labels, fontsize=11, ncol=2, frameon=False, columnspacing=0.5, handletextpad=-0.3,loc='lower center', bbox_to_anchor=(0.76, 0.95))

    plt.gca().add_artist(legend1)

    plt.ylim(bottom=0, top=1.05)
    plt.yticks([0, 0.2, 0.4, 0.6, 0.8, 1])
    plt.xticks([0, 6, 12, 18, 19])
    plt.gca().yaxis.set_major_formatter(mticker.PercentFormatter(1.0)) 
    plt.xlabel('Number of Application Cores', fontsize=11)
    plt.ylabel('Timer Core CPU Usage (%)', fontsize=11)
        
    plt.subplots_adjust(left=0.15, right=0.98, top=0.8, bottom=0.18) 
    plt.savefig(f'{RESULT_DIR}/figure1.pdf')
    plt.show()    
    
collect()
plot()

from string import Template
from matplotlib import pyplot as plt

junk_length = len("board.loadgen.LoadGenerator.latency::")

def open_and_find_latency(stat_file_name):
    global junk_length
    f = open(stat_file_name,'r')
    lines = f.readlines()
    loadgen_lines = []
    for line in lines:
        if "LoadGenerator" in line and "%" in line:
            line_without_junk = line[junk_length:]
            loadgen_lines.append(line_without_junk)
    latency_lines = []
    for line in loadgen_lines:
        latency_lines.append(line)
        if "100.00%" in line:
            break
    for line in reversed(latency_lines):
        vals = line.split()
        percent = float(vals[3][:-1])
        if percent > 95.0:
            up_95 = percent
            val_up = float(vals[0].split('-')[0])
        else:
            down_95 = percent
            val_down = float(vals[0].split('-')[0]) 
            break
    if down_95 > 94.98:
        val = val_down
    else:
        b = (95.0-down_95)/(up_95-95)
        val = (val_down + val_up*b)/(1+b)
    f.close()
    return val

stat_template = Template("stats/stat${load}GbpsTrack${interval}${int_type}")
loads = [0.5,1,1.5,2,2.5,3,3.5,4,4.5,5,6,9,12,15,18]
intervals = [1,10,50,100,200,300,400,500,600,700,800,900,1000,1500,2000]

for load in loads:
    stat_file_name = stat_template.substitute({
        "load": load,
        "interval": 0,
        "int_type": "PCI"
    })
    x_pci = [open_and_find_latency(stat_file_name)]*len(intervals)
    x_timed = []
    for interval in intervals:
        stat_file_name = stat_template.substitute({
            "load": load,
            "interval": 0,
            "int_type": "PCI"
        })
        x_timed.append(open_and_find_latency(stat_file_name))
    plt.plot(x_pci, intervals)
    plt.plot(x_timed,intervals)
    plt.show()

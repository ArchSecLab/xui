import os
from string import Template
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns
import pandas as pd
import altair as alt
import matplotlib.ticker as mtick
def set_style():
   #plt.style.use(['seaborn-white', 'seaborn-paper'])
   plt.rcParams["font.family"] = "serif"
#    plt.rcParams['pdf.fonttype'] = 42
#    plt.rcParams['ps.fonttype'] = 42
#    sns.set(rc={'figure.figsize':(11.7,8.27),"font.size":20, "font.family": "serif", 
            #    "axes.titlesize":20,"axes.labelsize":20, "ytick.labelsize":20, 
                # "xtick.labelsize":16 , 'legend.fontsize':18, 'legend.title_fontsize': 18}, style="white")
set_style()
def open_latency_file(packet_rate, int_type_full, interrupt_interval, pci_type, trial, nics):
    template = Template("latencyl3${packet_rate}Gbps${int_type_full}${interrupt_interval}${pci_type}${trial}Trials${nics}Nics.txt")
    filename = template.substitute(
        packet_rate=packet_rate,
        int_type_full=int_type_full,
        interrupt_interval=interrupt_interval,
        pci_type=pci_type,
        trial=trial,
        nics=nics
    )
    filepath = os.path.join("latencies/", filename)
    return open(filepath, 'r')
def split_and_sort_file(file):
    latencies = []
    other_info_lines = []
    
    for line in file:
        stripped_line = line.strip()
        if stripped_line.replace('.', '', 1).isdigit():
            latencies.append(float(stripped_line))
        else:
            other_info_lines.append(stripped_line)
    
    # latencies.sort()
    return {"latencies": latencies, "other_info_lines": other_info_lines}
ratio = None
def register_background_counter(other_info_lines):
    global ratio
    for line in other_info_lines:
        if line.startswith("Background cycles:"):
            counter = int(line.split(":")[1].strip())
            ratio = 62000000.0 / counter
def find_cycles(other_info_lines):
    net_cycles = 0
    poll_cycles = 0
    back_cycles = 0

    for line in other_info_lines:
        if line.startswith("Network cycles:"):
            net_cycles = int(line.split(":")[1].strip())
        elif line.startswith("Poll cycles:"):
            poll_cycles = int(line.split(":")[1].strip())
        elif line.startswith("Background cycles:"):
            back_cycles = int(line.split(":")[1].strip()) * ratio

    return net_cycles, poll_cycles, back_cycles
#we want to plot a bar graph of the cycles spent in the network, poll, and background
#per 1/ 4/ 8 NIC devices
max_packet_rate = {1:32, 4:31, 8:29} # max packet rate for 1/4/8 NICs
interrupt_intervals = [1,5] # interrupt intervals


timer_types = ["TIMER", "UIPI", "APIC"]
test_configs = []
num_nics = [1, 8] # number of NICs
trials = 5 # number of trials
class Types:
    device = "Track Dev UI"
    polling = "Polling"
class TypesWhenOpeningFiles:
    device = "PCI"
    polling = "SPIN"
interrupt_types = [Types.device, Types.polling]
interrupt_types_when_opening_files = ["PCI", "SPIN" ]
pci_type = {Types.device: "Track", Types.polling: "Track", "TIMER": "Track", "UIPI": "Flush", "APIC": "Apic"}


# for interrupt_interval in interrupt_intervals:
    # for timer_type in timer_types:
        # test_configs.append((interrupt_interval, timer_type))
# for interrupt_type in interrupt_types:
#     test_configs.append(interrupt_type))

import palettable
import operator as o
ylabel_set = False
def stacked_barplot(ax, dpoints1, ylabel):
    '''
    Create a barchart for data across different categories with
    multiple conditions for each category.
    
    @param ax: The plotting axes from matplotlib.
    @param dpoints: The data set as an (n, 3) numpy array
    '''
    global ylabel_set
    patterns = [ "+" , "x" , "\\" , "/", "o", "O", ".", "*" ]
    colors = ['#4daf4a', '#ff7f00', '#e41a1c','#377eb8',
                   
                  '#cccccc', '#dede00']
    #colors = palettable.colorbrewer.qualitative.Set1_6.mpl_colors
    #BlueRed_6.mpl_colors.get_map('Pastel2', 'qualitative', 8).mpl_colors#[(0.74,0.04,0.04), (0.0,0.62,0.001),(0.01,0,0.75)]
    # Aggregate the conditions and the categories according to their
    # mean values
    def uniq(l):
        indexes = np.unique(l, return_index=True)[1]
        return [l[index] for index in sorted(indexes)]
    conditions = [(c, np.mean(dpoints1[dpoints1[:,0] == c][:,2].astype(float)))
                  for c in uniq(dpoints1[:,0])]
    
    categories = [(c, dpoints1[dpoints1[:,1] == c][:,2].astype(float)) 
                  for c in uniq(dpoints1[:,1])]
    # np.mean(dpoints1[dpoints1[:,3] == c][:,2].astype(float))
    breakdowns = [(c, i) 
                  for i,c in enumerate(uniq(dpoints1[:,3]))]
    # sort the conditions, categories and data so that the bars in
    # the plot will be ordered by category and condition
    conditions.reverse()
    breakdowns.reverse()
    conditions = [c[0] for c in conditions]
    categories = [c[0] for c in categories]
    breakdowns = [c[0] for c in breakdowns]
    #dpoints1 = np.array(sorted(dpoints1, key=lambda x: categories.index(x[1])))

    
    
    n = len(conditions)
    # the space between each set of bars
    space = 1.6
    per_catg_width = 16.0 
    width = per_catg_width / n 
    offset = - per_catg_width/2.0 - 2*space
    if n==1:
        width = 5
        space = 1
        offset = 0 
    

    ax.grid( linestyle='-.', linewidth=0.5, alpha=.5, axis='y', zorder=0)
    # Create a set of bars at each position
    u= 0 
    positions = []
    covered_bd = []
    
    for i,cond in enumerate(conditions):
        indeces = range(1, len(categories)+1)
        oldvals = 0 
        pos = [(j+1)*space*per_catg_width + offset  + i * (width+1.5)+space for j in indeces]
        positions = positions + pos
        for x,bd in enumerate(breakdowns):
            mask = np.logical_and((dpoints1[:,0::3] == (cond,bd))[:,1],(dpoints1[:,0::3] == (cond,bd))[:,0])
            vals = dpoints1[mask][:,2].astype(np.cfloat)
            #pdb.set_trace() 
            if not bd in covered_bd and vals.any():
                lbl = bd
                covered_bd.append(bd)
            else:
                lbl = None #cond+"|"+bd
            if not vals.any():
                lbl = None
            ax.bar(pos, vals, width=width, label=lbl, bottom=oldvals,
                    edgecolor=colors[4],
                    hatch=3*patterns[min(x,7)],
                    color=colors[x],
                    alpha=0.65,
                    zorder=3)
            ax.bar(pos, vals, width=width, bottom=oldvals,
                    edgecolor='black',
                    color='none',
                    zorder=4)
            if  vals.any():
                u = u+1
            oldvals += vals
                 #  color=cm.Accent(float(i) / n))
            
    # Set the x-axis tick labels to be equal to the categories
    xticks = [None]*(n*len(categories))
    for i,cat in enumerate(categories):
        for j,cond in enumerate(conditions):
            if len(conditions) > 1:
                xticks[i*n+j] = cond
            else:
                xticks[i*n+j] = cat
    positions = sorted(positions) 
    ax.set_xticks(positions)
    if ylabel_set:
        ax.set_xticklabels(xticks,rotation=90, fontsize=13)
    else:
        ax.set_xticklabels(xticks,rotation=90, fontsize=13,visible=False)
        ax.tick_params(axis='x', which='both', left=False, right=False, labelleft=False)



    if len(conditions) > 1:
        line = plt.Line2D([positions[0]-width/2-2*space,positions[0]-width/2-2*space], [0 , -10.3], zorder = 10,
                   lw=1,color='black')
        line.set_clip_on(False)
        ax.add_line(line)
        cat_displace_y = 226.3
        line_len_y = -10.3
        character_size = 11
        multiplier = 0.09
        
        for i,cat in enumerate(categories):
            text_size = len(cat)*multiplier*character_size
            half_text_size = text_size/2
            if i == len(categories) - 1:
                xd = positions[(1+i)*n-1] + width/2 + space
                if ylabel_set:
                    ax.text(xd-width+2.8-space-half_text_size, cat_displace_y, cat, ha='center',fontsize=character_size)
            else:
                xd = (positions[(1+i)*n-1]+positions[(1+i)*n])/2
                if ylabel_set:
                    ax.text(xd-width-2*space-half_text_size, cat_displace_y, cat, ha='center',fontsize=character_size)
            #pdb.set_trace()
            line = plt.Line2D([xd, xd], [0 ,line_len_y], zorder = 10,
                       lw=1,color='black')
            
            line.set_clip_on(False)
            ax.add_line(line)
        

        
#    plt.setp(plt.xticks()[1], rotation=45)
    
    # Add the axis labels
    
    # ax.set_ylabel(ylabel, fontsize=14)
    handles, labels = ax.get_legend_handles_labels()
    if not ylabel_set:
        ax.legend(handles[::-1], labels[::-1], loc=(0.05,1.23), edgecolor='black', ncol=2,fontsize=14)
        ylabel_set = True
    #ax.set_xlabel("Structure")

    
    # Add a legend
    
        
    # ax.spines['top'].set_visible(False)
    # ax.spines['right'].set_visible(False)

# per nic configuration %0 %20 %40 %60 %80 %100 cycle distributions
per_nic_results = {1:{}, 4:{}, 8:{}}
for nic in num_nics:
    for test_type in interrupt_types:
        per_nic_results[nic][test_type] = {
            "Network": [0, 0, 0, 0, 0, 0],
            "Poll": [0, 0, 0, 0, 0, 0],
            "Background": [0, 0, 0, 0, 0, 0]
        }
import pickle
# check if we have per_nic_results.pkl
not_saved = True
if False and os.path.exists("per_nic_results.pkl"):
    with open("per_nic_results.pkl", "rb") as f:
        per_nic_results = pickle.load(f)
        not_saved = False
    print("Loaded per_nic_results.pkl")
else:
    vacant = open_latency_file(0, "Track", 0, TypesWhenOpeningFiles.device, 5, 1)
    vacant_info = split_and_sort_file(vacant)["other_info_lines"]
    register_background_counter(vacant_info)
    vacant.close()
    print(f"Ratio: {ratio}")
    cycle_percentage = 100.0/62000000.0
    loads = ["%0 Load", "%20 Load", "%40 Load", "%60 Load", "%80 Load", "%100 Load"]
    bar_names = ["Networking Cycles", "Polling Overhead", "Interrupt Overhead", "Rescued Cycles"]
    fig, axes = plt.subplots(2, 1, figsize=(8.25, 5))
    fig.subplots_adjust(wspace=0.02)
    char_size_annote = 16
    char_scale = 0.0054

    for ax, col in zip(axes, [r"1 queue",r"8 queues"]):
        text_size = len(col)*char_scale*char_size_annote
        half_text_size = text_size*0.5
        print(f"Text size: {text_size}")
        ax.annotate(col, xy=(1.035,0.5-half_text_size), xytext=(0, 5),
                    xycoords='axes fraction', textcoords='offset points',
                    ha='center', va='bottom',fontsize=16,rotation=90)
    # One Ylabel for both plots
    fig.text(0.02, 0.5, 'Percentage over all Cycles', va='center', rotation='vertical', fontsize=16)
    for i,nic in enumerate(num_nics):
        ax = axes[i]
        dpoint = np.zeros((0,4))
        if i == 0 or True:
            ax.set_yticks(np.arange(0,100.03,10))
            ax.set_ylim(bottom=0,top=100.05)
            ax.yaxis.set_major_formatter(mtick.PercentFormatter())
            ax.tick_params(axis='y', which='both', labelsize=12)
        #else:
        #    ax.set_yticks(np.arange(0,100.03,10))
        #    ax.tick_params(axis='y', which='both', left=False, right=False, labelleft=False)
        #    ax.set_ylim(bottom=0,top=100.05)
        #    ax.yaxis.set_major_formatter(mtick.PercentFormatter())
        
        per_nic_results[nic][Types.device]["Background"][0] = 100.0
        per_nic_results[nic][Types.polling]["Poll"][0] = 100.0
        
        
        for i,test_config in enumerate(interrupt_types):
            int_type = test_config
            interrupt_interval = 0
            for load in range(0, 6):
                if load == 0 and int_type == Types.device:
                    dpoint = np.concatenate( (dpoint,[[Types.device, loads[0], 0, bar_names[0]]]))
                    dpoint = np.concatenate( (dpoint,[[Types.device, loads[0], 0, bar_names[1]]]))
                    dpoint = np.concatenate( (dpoint,[[Types.device, loads[0], 0, bar_names[2]]]))
                    dpoint = np.concatenate( (dpoint,[[Types.device, loads[0], 100, bar_names[3]]]))
                    continue
                elif load == 0 and int_type == Types.polling:
                    dpoint = np.concatenate( (dpoint,[[Types.polling, loads[0], 0, bar_names[0]]]))
                    dpoint = np.concatenate( (dpoint,[[Types.polling, loads[0], 100, bar_names[1]]]))
                    dpoint = np.concatenate( (dpoint,[[Types.polling, loads[0], 0, bar_names[2]]]))
                    dpoint = np.concatenate( (dpoint,[[Types.polling, loads[0], 0, bar_names[3]]]))
                    continue
                file = open_latency_file(max_packet_rate[nic]*load*0.2, pci_type[int_type], interrupt_interval, interrupt_types_when_opening_files[i], 5, nic)
                split_file = split_and_sort_file(file)
                latencies = split_file["latencies"]
                other_info_lines = split_file["other_info_lines"]
                net_cycles, poll_cycles, back_cycles = find_cycles(other_info_lines)
                poll_cycles = (62000000.0 - net_cycles - back_cycles)* cycle_percentage
                net_cycles = net_cycles * cycle_percentage
                back_cycles = back_cycles * cycle_percentage
                file.close()
                # total_cycles = net_cycles + poll_cycles + back_cycles
                # per_nic_results[nic][test_config]["Network"][load] = net_cycles * cycle_percentage
                # per_nic_results[nic][test_config]["Poll"][load] = poll_cycles * cycle_percentage
                # per_nic_results[nic][test_config]["Background"][load] = back_cycles * cycle_percentage
                dpoint = np.concatenate( (dpoint,[[test_config, loads[load], net_cycles, bar_names[0]]]))
                if test_config == Types.device:
                    dpoint = np.concatenate( (dpoint,[[test_config, loads[load], 0, bar_names[1]]]))
                    dpoint = np.concatenate( (dpoint,[[test_config, loads[load], poll_cycles, bar_names[2]]]))
                else:
                    dpoint = np.concatenate( (dpoint,[[test_config, loads[load], poll_cycles, bar_names[1]]]))
                    dpoint = np.concatenate( (dpoint,[[test_config, loads[load], 0, bar_names[2]]]))
                dpoint = np.concatenate( (dpoint,[[test_config, loads[load], back_cycles, bar_names[3]]]))
        stacked_barplot(ax,dpoint, "Percentage over all cycles")
        
        # set legend above the plotax.lege
        if i == len(num_nics) - 1:
            ax.set_title("")
    # plt.tight_layout()
    plt.savefig(f"fig-l3-forwarding.pdf",bbox_inches='tight')


if not_saved:
    with open("per_nic_results.pkl", "wb") as f:
        pickle.dump(per_nic_results, f)
        print("Saved per_nic_results.pkl")
exit(0)

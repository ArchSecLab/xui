
from string import Template
from matplotlib import pyplot as plt
from matplotlib.lines import Line2D
import numpy as np
import seaborn as sns
import pandas as pd
import matplotlib.ticker as mtick

junk_length = len("board.loadgen.LoadGenerator.latency::")
max_val = -1
def open_and_find_latency(lat_file_name):
    l = open(lat_file_name)
    lines = l.readlines()
    # Uncached read causes the first read to be slower and distorts the results
    lines = lines[1:]
    
    lines_as_numbers = sorted([float(i)*0.0005 for i in lines])
    return lines_as_numbers

stat_template = Template("stats/stat${load}GbpsTrack${interval}${int_type}${mode_type}.txt")
stat_template_apic = Template("stats/stat${load}GbpsApic${interval}${int_type}${mode_type}.txt")
stat_template_uipi = Template("stats/stat${load}GbpsFlush${interval}${int_type}${mode_type}.txt")
stat_template_2 = Template("stats/stat${load}GbpsTrack${interval}${int_type}${mode_type}.txt")
stat_template_2_apic = Template("stats/stat${load}GbpsApic${interval}${int_type}${mode_type}.txt")
lat_template = Template("latencies/latency${req}usTrack${stddev}${int_type}${max_err}Half.txt")
lat_template_apic = Template("latencies/latency${req}usApic${stddev}${int_type}${max_err}Half.txt")
lat_template_uipi = Template("latencies/latency${req}usFlush${stddev}${int_type}${max_err}Half.txt")
board_template = Template("boards/board${req}usTrack${stddev}${int_type}${max_err}Half")
board_template_apic = Template("boards/board${req}usApic${stddev}${int_type}${max_err}Half")
board_template_uipi = Template("boards/board${req}usFlush${stddev}${int_type}${max_err}Half")
request_lengths = [2,5,10,20,30,50,70,100]
max_errors = [1]
stddevs = [0.2739, 0.3873, 0.4743, 0.5477, 0.6124, 0.866, 1.2247, 1.5]
stddevs = [0.0,0.2,0.5,0.8,1.1,1.4,1.7,2.0]#[0.2,0.4,0.6,0.8,1.0,1.5,2.0]
stddevs = [0.0,0.3,0.65,0.72,0.95,1.02,1.25,1.32,1.7,2.0]
request_lengths = {1:[2],10:[20]}
min_errors = [1,10]
CB_color_cycle = ['#377eb8', '#ff7f00', '#4daf4a',
                  '#f781bf', '#a65628', '#984ea3',
                  '#999999', '#e41a1c', '#dede00']
plt.rcParams['axes.prop_cycle'] = plt.cycler(color=CB_color_cycle)
plt.rcParams['font.family'] = 'serif'
def open_and_find_extra(stat_file_name):
    global junk_length
    try:
        f = open(stat_file_name,'r')
    except:
        print("we dont have: "+stat_file_name)
        return 0
    lines = f.readlines()
    loadgen_lines = []
    sent = None
    for line in reversed(lines):
        if "Extra work" in line:
            sent = float(line.split()[3])
            break
    if sent is None:
        print(stat_file_name)
        print(lines[-1])
        sent = 0
    f.close()
    return sent * 34.11121576774443 / 3000
def open_and_find_run_time(board_file_name):
    try:
        f = open(board_file_name,'r')
    except:
        print("we dont have: "+board_file_name)
        return 0
    lines = f.readlines()
    time = None
    for line in reversed(lines):
        if "Time spent:" in line:
            time = float(line.split()[2])
            break

    if time is None:
        print(board_file_name)
        time = 100000000000000000000000
    f.close()
    return time/3000
def open_and_find_ratio(stddev, max_err, req):
    global request_lengths
    total_times = {}
    stddevs = [0.2,2.0,5.0, 10.0,15.0]
    for min_err in [1,10]:
        total_times[min_err] = {}
        for stddev_x in stddevs:
            total_times[min_err][stddev_x] = {}
            for reqx in request_lengths[min_err]:
                total_times[min_err][stddev_x][reqx] = 0
    source_str = """
    Stddev:0.2 Request Length:2 Min Error:1 Total:6004.55
    Stddev:0.2 Request Length:20 Min Error:10 Total:59994.2
    Stddev:2 Request Length:2 Min Error:1 Total:7727.65
    Stddev:2 Request Length:20 Min Error:10 Total:60045.5
    Stddev:5 Request Length:2 Min Error:1 Total:10210.6
    Stddev:5 Request Length:20 Min Error:10 Total:62664.7
    Stddev:10 Request Length:2 Min Error:1 Total:13203.9
    Stddev:10 Request Length:20 Min Error:10 Total:68771.6
    Stddev:15 Request Length:2 Min Error:1 Total:15549.2
    Stddev:15 Request Length:20 Min Error:10 Total:73168.5
        """
    source_str = source_str.split("\n")
    for line in source_str:
        if line == "":
            continue
        parts = line.split()
        if parts == []:
            continue
        if parts[0] == "=":
            continue
        try:
            stddev_not_param = float(parts[0].split(":")[1])
        except:
            print(parts)
            print(parts[0])
        req_not_param = int(parts[2].split(":")[1])
        min_err = int(parts[4].split(":")[1])
        total = float(parts[5].split(":")[1])
        total_times[min_err][stddev_not_param][req_not_param] = total
    if total_times[max_err][stddev][req] == 0:
        print("We dont have the total time for "+str(req)+" "+str(stddev)+" "+str(max_err))
    try:
        return total_times[max_err][stddev][req]/3000
    except:
        print("We dont have the total time for "+str(req)+" "+str(stddev)+" "+str(max_err))
        return 0
    
def find_plot_info(int_type, max_err, req, stddev, xs,stddevs,base,is_apic=False):
    subs = {
                "req":req,    
                "stddev": stddev,
                "int_type": int_type,
                "max_err": max_err
            }
    
    lat_file_name = lat_template.substitute(subs)
    if is_apic:
        lat_file_name = lat_template_apic.substitute(subs)
    x_ = open_and_find_latency(lat_file_name)
    xs.extend(x_)#[100*(x-base[i])/base[i] for i,x in enumerate(x_)])

    stddevs.extend([stddev]*len(x_))
# We are shifting to seaborn lineplot
fig, axes = plt.subplots(2, 2, figsize=(9, 7))

fig.subplots_adjust(wspace=0.26, hspace=0.4)
for ax, col in zip(axes[0], [r"2$\mu s$ requests",r"20$\mu s$ requests"]):
    ax.annotate(col, xy=(0.5, -1.78), xytext=(0, 5),
                xycoords='axes fraction', textcoords='offset points',
                ha='center', va='bottom',fontsize=16)
for i,max_err in enumerate(min_errors):
    ax = axes[1][i]
    # ax.yaxis.set_major_formatter(mtick.PercentFormatter())
    for j,req in enumerate(request_lengths[max_err]):
        
        lat_shifts = {"timer":[],"pci":[],"spin":[],"uipi":[],"apic":[]}
        x_apic_flat = []
        x_pci_flat = []
        x_spin_flat = []
        x_uipi_flat = []
        x_timed_flat = []
        stddevs_flat_apic = []
        stddevs_flat_pci = []
        stddevs_flat_spin = []
        stddevs_flat_uipi = []
        stddevs_flat_timed = []
        for stddev in stddevs:
            # supposed_to_take = open_and_find_ratio(stddev, max_err, req)
            subs = {
                "req":req,    
                "stddev": stddev,
                "int_type": "SPIN",
                "max_err": max_err
            }
            board_file_name = board_template.substitute(subs)
            x_spin = open_and_find_latency(lat_template.substitute(subs))
            find_plot_info("SPIN",max_err,req,stddev,x_spin_flat,stddevs_flat_spin,x_spin)
            find_plot_info("TIMER",max_err,req,stddev,x_timed_flat,stddevs_flat_timed,x_spin)
            find_plot_info("PCI",max_err,req,stddev,x_pci_flat,stddevs_flat_pci,x_spin)
            find_plot_info("APIC",max_err,req,stddev,x_apic_flat,stddevs_flat_apic,x_spin,is_apic=True)
            # subs = {
            #     "req":req,    
            #     "stddev": stddev,
            #     "int_type": "UIPI",
            #     "max_err": max_err
            # }
            # lat_file_name = lat_template_uipi.substitute(subs)
            # x_uipi = open_and_find_latency(lat_file_name)
            # x_uipi_flat.extend([x for x in x_uipi])
            # if len(x_uipi) != 0:
            #     stddevs_flat_uipi.extend([stddev]*len(x_uipi))
            # #x_uipi = x_uipi[:-1]
            # x_uipi_avg = sum(x_uipi)/len(x_uipi) 
            # lat_shifts["uipi"].append(x_uipi_avg-req)

            
            #bin_length = (x_max)*0.03
            #plt.xlabel("Latency in us")
            #plt.ylabel("Number of packets")
            #plt.title("Req:"+str(req)+"Variance/Mean: "+str(vmr)+" Max % difference then mean request time:"+str(max_err*100))
            #plt.hist(x_uipi,bins=3,alpha=0.5,label="UIPI")
            #def hist(x,label):
            #    dist = x[-1] - x[0]
            #    print(dist)
            #    print(bin_length)
            #    bin_count = round(dist/bin_length)
            #    plt.hist(x,bins=bin_count,alpha=0.5,label=label)
            #hist(x_timed,label="LEAN")
            #hist(x_uipi,label="UIPI")
            #hist(x_apic,label="APIC")
            #hist(x_pci,label="PCI")
            #hist(x_spin,label="SPIN")

        #flatten for seaborn
        stddevs_flat = []
        for i,stddev in enumerate(stddevs):
            stddevs_flat.extend([stddev]*2999)
        # Create a DataFrame in long form
        df_timed = pd.DataFrame({
            'x': stddevs_flat_timed,
            'y': x_timed_flat,
        })

        # df_uipi = pd.DataFrame({
        #     'x': stddevs_flat_uipi,
        #     'y': x_uipi_flat,
        # })

        df_apic = pd.DataFrame({
            'x': stddevs_flat_apic,
            'y': x_apic_flat,
        })

        df_pci = pd.DataFrame({
            'x': stddevs_flat_pci,
            'y': x_pci_flat,
        })

        df_spin = pd.DataFrame({
            'x': stddevs_flat_spin,
            'y': x_spin_flat,
        })
        # Plot using seaborn lineplot
        plots = [None]*7
        labels = ["KBTimer", "itimer", r"Device Uintr", "Busy Spinning"]
        # plots[5], = sns.lineplot(data=[],label="Periodic")
        # plots[6], = sns.lineplot(data=[],label="")
        plots[0] = sns.lineplot(ax=ax,data=df_timed, x='x', y='y',markers='*',label="Lean Timer",legend=False)
        # plots[1] = sns.lineplot(data=df_uipi, x='x', y='y',markers='*',label="Uipi Software Timer")
        plots[1] = sns.lineplot(ax=ax,data=df_apic, x='x', y='y',markers='*',label="itimer",legend=False)
        plots[2] = sns.lineplot(ax=ax,data=df_pci, x='x', y='y',markers='x',label="Device Uintr",legend=False)
        plots[3] = sns.lineplot(ax=ax,data=df_spin, x='x', y='y',markers='o',label="Busy Spinning",legend=False)

        if req == 2 and max_err == 1:
            pos_periodic = 'upper left'
        else:
            pos_periodic = 'upper left'
        
        if req == 2 and max_err == 1:
            pos = 'lower right'
        else:
            pos = 'lower right'
        #legend2 = plt.legend(handles=plots[0].lines[2:],labels=labels[2:],loc=pos,fontsize=15)
        #ax.add_artist(legend)
        y_lim = ax.get_ylim()
        ax.set_ylim((y_lim[0],y_lim[1]))
        if req == 20 and max_err == 10:
            #ax.set_ylim((0,30))
            pass
        if req == 2 and max_err == 1:
            
            pass
        ax.set_ylim((0,y_lim[1]))
        #ax.set_xlim((0,16))
        ax.set_xlabel("Unpredictability (StdDev)",fontsize=14)
        ax.set_ylabel(r"Latency $\mu s$",fontsize=14)
        ax.tick_params(which='both',labelsize=12)
        #plt.plot(stddevs,lat_shifts["timer"], label="Lean Timer",marker='o')
        #plt.plot(stddevs,lat_shifts["apic"], label="Apic Timer",marker='o')
        #plt.plot(stddevs,lat_shifts["uipi"], label="Uipi Software Timer",marker='o')
        #plt.plot(stddevs,lat_shifts["pci"], label="Fast Device Interrupt",marker='o')
        #plt.plot(stddevs,lat_shifts["spin"], label="Busy Spin",marker='o')
        

        
        #plt.show()

max_pci_ratio = 0
configs_on_max_pci = {"stddev":0, "req":0, "max_err":0}
for i,max_err in enumerate(min_errors):
    ax = axes[0][i]
    for req in request_lengths[max_err]:
    
        x_pci = []
        x_timed = []
        x_spin = []
        x_apic = []
        x_uipi = []
        req_c = req * 2000
        for stddev in stddevs:
            subs = {
                "req":req,    
                "stddev": stddev,
                "int_type": "PCI",
                "max_err": max_err
            }
            b_file_name = board_template.substitute(subs)
            x_pci.append(1-(open_and_find_extra(b_file_name)/open_and_find_run_time(b_file_name)))
            # x_pci.append(open_and_find_extra(b_file_name))
            ref = open_and_find_extra(b_file_name)
            subs = {
                "req":req,    
                "stddev": stddev,
                "int_type": "TIMER",
                "max_err": max_err
            }
            b_file_name = board_template.substitute(subs)
            lat_file_name = lat_template.substitute(subs)
            x_timed.append(1-(open_and_find_extra(b_file_name)/open_and_find_run_time(b_file_name)))
            # x_timed.append(open_and_find_extra(b_file_name))
            subs = {
                "req":req,    
                "stddev": stddev,
                "int_type": "SPIN",
                "max_err": max_err
            }
            b_file_name = board_template.substitute(subs)
            # lat_file_name = lat_template.substitute(subs)
            # expected = open_and_find_expected(lat_file_name)
            # req_c = expected * 2000
            x_spin.append(1-(open_and_find_extra(b_file_name)/open_and_find_run_time(b_file_name)))
            # x_spin.append(open_and_find_extra(b_file_name))
            subs = {
                "req":req,    
                "stddev": stddev,
                "int_type": "APIC",
                "max_err": max_err
            }
            b_file_name = board_template_apic.substitute(subs)
            # lat_file_name = lat_template.substitute(subs)
            # expected = open_and_find_expected(lat_file_name)
            # req_c = expected * 2000
            x_apic.append(1-(open_and_find_extra(b_file_name)/open_and_find_run_time(b_file_name)))
            # x_apic.append(open_and_find_extra(b_file_name))
            subs = {
                "req":req,    
                "stddev": stddev,
                "int_type": "UIPI",
                "max_err": max_err
            }
            #b_file_name = board_template_uipi.substitute(subs)
            # lat_file_name = lat_template.substitute(subs)
            # expected = open_and_find_expected(lat_file_name)
            # req_c = expected * 2000
            #x_uipi.append(1-(open_and_find_extra(b_file_name)/open_and_find_run_time(b_file_name)))
        ax.set_xlabel("Unpredictability (StdDev)",fontsize=14)
        ax.set_ylabel("Norm. Wasted Cycles",fontsize=14)
        # plt.title("Accelerator request Time:"+str(req)+"us with 1us noise")
            #plt.hist(x_uipi,bins=3,alpha=0.5,label="UIPI")
            #plt.hist(x_uipi,bins=3,alpha=0.5,label="UIPI")
            #plt.hist(x_uipi,bins=3,alpha=0.5,label="UIPI")
        df_pci = pd.DataFrame(data={"x":stddevs,"y":x_pci})
        df_timed = pd.DataFrame(data={"x":stddevs,"y":x_timed})
        df_spin = pd.DataFrame(data={"x":stddevs,"y":x_spin})
        df_apic = pd.DataFrame(data={"x":stddevs,"y":x_apic})
        #df_uipi = pd.DataFrame(data={"x":stddevs,"y":x_uipi})
        plots = [None]*7
        labels = [r"UI KB Timer + Tracking",  "itimer", r"Tracked Device UIs", "Polling"]
        # plots[5], = sns.lineplot(data=[],label="Periodic")
        # plots[6], = sns.lineplot(data=[],label="")
        plots[0] = sns.lineplot(ax=ax,data=df_timed, x='x', y='y',label="Lean Timer",legend=False)
        # plots[1] = sns.lineplot(data=df_uipi, x='x', y='y',label="Uipi Software Timer")
        plots[1] = sns.lineplot(ax=ax,data=df_apic, x='x', y='y',label="itimer",legend=False)
        plots[2] = sns.lineplot(ax=ax,data=df_pci, x='x', y='y',label="Device Uintr",legend=False)
        plots[3] = sns.lineplot(ax=ax,data=df_spin, x='x', y='y',label="Busy Spinning",legend=False)
    
        y_lim = ax.get_ylim()
        x_lim = ax.get_xlim()
        ax.set_ylim((0,y_lim[1]))
        ax.tick_params(which='both',labelsize=12)
        
        # ax.set_xlim((0,x_lim[1]))
        #ax.set_ylim((y_lim[0],0.35))

        # ax.set_xlim((0,16))
        #plt.plot(request_lengths,x_spin,label="SPIN")
        





# plt.xticks(fontsize=15)
# plt.yticks(fontsize=15)

legend = plt.legend(handles=plots[0].lines,labels=labels, loc=(-1.2,2.46),fontsize=15,title_fontsize=15,ncol=2)
# fig.
# plt.tight_layout()
plt.savefig("fig_accelerator_repr.pdf")
    #plt.title("Loa

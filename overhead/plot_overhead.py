from string import Template
from matplotlib import pyplot as plt
from matplotlib.lines import Line2D
import numpy as np
import seaborn as sns
import pandas as pd
import matplotlib.ticker as mtick


CB_color_cycle = ['#e41a1c','#4daf4a', '#ff7f00', '#377eb8',
                   
                  '#cccccc', '#dede00']
plt.rcParams['axes.prop_cycle'] = plt.cycler(color=CB_color_cycle)
plt.rcParams['font.family'] = 'serif'

out_template = Template('outs/out${int_type}${interval}_hard${hard}_${test}')
interrupt_intervals = [1,3,5,10,20,50,100]
test_names = ["fib","linpack","memops"]
non_hard_int_types = ['Flush','Track']
baseline_interval = 140000000
"""
number_print_from_gem5: 0
number_print_from_gem5: total_time
number_print_from_gem5: 0
number_print_from_gem5: interrupt_count
"""
int_type_remapping = {'Flush':'UIPI SW Timer','Track':'UIPI SW Timer +\n Tracking','KBTimer':'UI KB Timer + Tracking'}
def get_time(subs):
    out = out_template.substitute(subs)
    with open(out) as f:
        lines = f.readlines()
        number_of_seperator_zeros = 0
        for line in lines:
            if "number_print_from_gem5: 0" in line:
                number_of_seperator_zeros += 1
            if "number_print_from_gem5: " in line and "number_print_from_gem5: 0" not in line and number_of_seperator_zeros == 1:
                return float(line.split()[1])


df = pd.DataFrame(columns=['Test Type','interval','overhead','Interrupt Type'])
baseline_times = {}
for test in test_names:
    subs = {'int_type':'Track', 'interval':baseline_interval, 'hard':1, 'test':test}
    baseline_times[test] = get_time(subs)
    for interval in interrupt_intervals:
        for non_hard_int_type in non_hard_int_types:
            subs = {'int_type':non_hard_int_type, 'interval':interval, 'hard':0, 'test':test}
            time = get_time(subs)
            df = df._append({'Test Type':test, 'interval':interval, 'overhead':((time/baseline_times[test])-1)*100, 'Interrupt Type':int_type_remapping[non_hard_int_type]}, ignore_index=True)
        subs = {'int_type':'Track', 'interval':interval, 'hard':1, 'test':test}
        time = get_time(subs)
        df = df._append({'Test Type':test, 'interval':interval, 'overhead':((time/baseline_times[test])-1)*100, 'Interrupt Type':int_type_remapping['KBTimer']}, ignore_index=True)

#df_tracking_hard_timer = df[df['int_type'] == 'TrackHardTimer']
#df_drain = df[df['int_type'] == 'Drain']
#df_flush = df[df['int_type'] == 'Flush']
#df_tracking = df[df['int_type'] == 'Track']
print(df[df['interval']==5])
# now plot interval vs overhead for each test
plot_scale = 1.1
#plt.figure(figsize=(5*plot_scale, 3*plot_scale))
#sns.lineplot(x='interval', y='overhead', hue='Interrupt Type', markers=True, style='Test Type',dashes=False, data=df)
#ax = plt.gca()
#ax.yaxis.set_major_formatter(mtick.PercentFormatter())
#plt.legend(fontsize=18)
#plt.xscale('log')
#plt.xlabel(r'Interrupt Interval ($\mu s$)',fontsize=18)
#plt.ylabel('Overhead (%)',fontsize=18)
#plt.xticks(fontsize=16)
#plt.yticks(fontsize=16)
#plt.grid(True)
#plt.tight_layout()
#plt.savefig('fig_overhead_vs_interval_.pdf')
#plt.clf()
fig, axes = plt.subplots(1, 3, figsize=(15*plot_scale, 3*plot_scale))
fig.subplots_adjust(wspace=0.05*plot_scale)
for ax,col in zip(axes, test_names):
    ax.annotate(col, xy=(0.5, 1.01), xytext=(0, 5),
                    xycoords='axes fraction', textcoords='offset points',
                    ha='center', va='bottom',fontsize=20)
labels = ['UIPI SW Timer','UIPI SW Timer +\n Tracking','UI KB Timer + Tracking']
for i,test in enumerate(test_names):
    df_test = df[df['Test Type'] == test]
    ax = axes[i]
    p = sns.lineplot(ax=ax,x='interval', y='overhead', hue='Interrupt Type', dashes=False, data=df_test,legend=False,alpha=0.5)
    ax.yaxis.set_major_formatter(mtick.PercentFormatter())
    ax.set_xscale('log')
    ax.xaxis.set_major_formatter(mtick.ScalarFormatter())
    major_ticks = np.arange(0, 50, 10)
    ax.set_yticks(major_ticks)
    if i ==0:
        ax.legend(handles=p.lines,labels=labels,fontsize=20, loc=(0.09,1.17),ncol=3)
        ax.set_xlabel(r'Interrupt Interval ($\mu s$)',fontsize=20)
        ax.set_ylabel('Program Slowdown (%)',fontsize=20)
        # ax.tick_params(axis='y', which='both', left=False, right=False, labelleft=False)
    else:
        ax.set_xlabel(r'Interrupt Interval ($\mu s$)',fontsize=20)
        ax.set_ylabel('',fontsize=20)
        ax.tick_params(axis='y', which='both', left=False, right=False, labelleft=False)

    ax.tick_params(axis='both', which='major', labelsize=18)

    # ax.grid(which='both')
    ax.grid(axis='y', linestyle='--', linewidth=2,markevery=10)

    ax.grid(axis='x', linestyle='solid', linewidth=2)


    # g.set_linestyle('dotted')
    # g.lines.set_markevery(10)
# plt.tight_layout()
plt.savefig('fig_overhead_vs_interval.pdf',bbox_inches='tight')

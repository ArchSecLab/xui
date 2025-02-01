import subprocess
import re
def get_tests():
    processes = subprocess.check_output('ps aux | grep test ', shell=True)
    process_split = processes.split(b'\n')
    process_split_non_grep = []
    for process in process_split:
        if b'grep' not in process:
            process_str = process.decode('ascii')
            if process_str == '':
                continue
            process_split_non_grep.append(process_str)
    test_cmds = [process.split()[11] for process in process_split_non_grep]
    return test_cmds


def get_test_values(test):
    test_gist = test[len('done'):]
    test_value_load = test_gist[:test_gist.find('Gbps')]
    test_value_interval = test_gist[len(test_value_load)+len('GbpsTrack'):]
    test_value_interval_timer = test_value_interval.find('TIMER')
    if test_value_interval_timer == -1:
        test_value_interval_pci = test_value_interval.find('PCI')
        if test_value_interval_pci == -1:
            test_value_interval_sig = test_value_interval.find('SIG')
            test_value_mode_str = test_value_interval[test_value_interval_sig+3:]
            test_value_interval = test_value_interval[:test_value_interval_sig]
            test_value_strat = 2
        else:
            test_value_mode_str = test_value_interval[test_value_interval_pci+3:]
            test_value_interval = test_value_interval[:test_value_interval_pci]
            test_value_strat = 1
    else:
        test_value_mode_str = test_value_interval[test_value_interval_timer+3:]
        test_value_interval = test_value_interval[:test_value_interval_timer]
        test_value_strat = 0
    if test_value_mode_str == 'low':
        test_value_mode = 1
    elif test_value_mode_str == 'eq':
        test_value_mode = 2
    else:
        test_value_mode = 3
    return ("T",int(test_value_load),int(test_value_interval),test_value_strat,10,test_value_mode)
    
    

def get_test_cases_in(test_cmd):
    doneStrs = []
    with open(test_cmd, 'r') as f:
        
        line = True
        while line:
            line = f.readline()
            if '/done' in line:
                test = line[len('touch ../../'):][:-1]
                
                doneStrs.append(test)
    return doneStrs

def get_done_tests():
    files = subprocess.check_output('ls -l', shell=True)
    files = files.split(b'\n')
    doneFiles = []
    for file in files:
        if b'done' in file:
            fileStr = file.decode('ascii')
            if fileStr == '':
                continue
            doneFiles.append(fileStr.split()[-1])
    return doneFiles

def get_undones():
    test_cmds = get_tests()
    done_tests = get_done_tests()
    undone_tests = []
    for test_cmd in test_cmds:
        cases = get_test_cases_in(test_cmd)
        for case in cases:
            if case not in done_tests:
                undone_tests.append(case)
    return undone_tests


from string import Template
import sys
with open("test_parallel_dpdk.sh", "r") as f:
	src = Template(f.read())
def substition_vals(short_form:str, packet_rate: int,interrupt_interval: int,pci:int,alt:int=10,mode:int=1):
	if pci ==1:
		pci_type = "PCI"
	elif pci == 2:
		pci_type = "SIG"
	else:
		pci_type = "TIMER"
	if mode == 1:
		mode_type = "low"
	if mode == 2:
		mode_type = "eq"
	if mode == 3:
		mode_type = "high"
	if short_form == 'N':
		return {
			"int_type": "N",
			"packet_rate": packet_rate,
			"interrupt_interval": interrupt_interval,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "NoInterrupt",
			"alt": alt,
			"mode":mode,
			"mode_type": mode_type
			}
	if short_form == "F":
		return {
			"int_type": "Flush",
			"packet_rate": packet_rate,
			"interrupt_interval": interrupt_interval,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "Flush",
			"alt": alt,
			"mode":mode,
			"mode_type": mode_type
			}
	if short_form == "D":
		return {
			"int_type": "Drain",
			"packet_rate": packet_rate,
			"interrupt_interval": interrupt_interval,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "Drain",
			"alt": alt,
			"mode":mode,
			"mode_type": mode_type
			
			}
	if short_form == "T":
		return {
			"int_type": "Track",
			"packet_rate": packet_rate,
			"interrupt_interval": interrupt_interval,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "Track",
			"alt": alt,
			"mode":mode,
			"mode_type": mode_type
			
			}
	return {}

def get_undones_2():
	job_list = []
	packet_rates2 = [10,14,18,20,28,30,32,34]
	for packet_rate in packet_rates2:
		job_list.append(("T",packet_rate,0,1,10,2))
		job_list.append(("T",packet_rate,25,0,10,2))
		job_list.append(("T",packet_rate,0,1,10,1))
		job_list.append(("T",packet_rate,25,0,10,1))
	packet_rates3 = [10,12,14,18,20,22]
	for packet_rate in packet_rates3:
		job_list.append(("T",packet_rate,0,1,10,3))
		job_list.append(("T",packet_rate,25,0,10,3))
	packet_rates1 = [38,42,46,50,54]
	for packet_rate in packet_rates1:
		job_list.append(("T",packet_rate,0,1,10,1))
		job_list.append(("T",packet_rate,25,0,10,1))
	needs = []
	templ = Template("done${packet_rate}GbpsTrack${interrupt_interval}${pci_type}${mode_type}")
	pci_types = ['TIMER','PCI','SIG']
	mode_types = [0,'low','eq','high']
	for job in job_list:
		needs.append(templ.substitute({
			"packet_rate": job[1],
			"interrupt_interval": job[2],
			"pci_type": pci_types[int(job[3])],
			"mode_type": mode_types[int(job[5])]
		}))
	dones = get_done_tests()
	undones = []
	if len(dones) == 0:
		return needs
	for one in needs:
		if one not in dones:
			undones.append(one)
	return undones
def job_undones():
    
    undones =get_undones_2()
    job_list = [get_test_values(undone) for undone in undones]

    return job_list

start_line = "#!/bin/bash\n"
job_list = job_undones()





packet_rates = [8,16,24,32,36,40,44,48,52,56,60,80,100,120]
interrupt_intervals = [25,300]
alterations = [10]
modes = [1,2,3]
#HERE
for mode in modes:
	for alt in alterations:
		for packet_rate in packet_rates:
			job_list.append(("T",packet_rate,0,1,alt,mode))
			for interrupt_interval in interrupt_intervals:
				job_list.append(("T",packet_rate,interrupt_interval,0,alt,mode))
				job_list.append(("T",packet_rate,interrupt_interval,2,alt,mode))
#print(job_list)
tests = [start_line] * 96
directory = 0
overalls = 0
while len(job_list):
	j = job_list.pop()
	tests[directory] += src.substitute(substition_vals(j[0],j[1],j[2],j[3],j[4],j[5]))+"\n"
	directory += 1
	overalls += 1
	if directory == 96:
		directory = 0

for i in range(0,96):
	with open("test_parallel"+str(i)+".sh", "w") as f:
		f.write(tests[i])
print(overalls)
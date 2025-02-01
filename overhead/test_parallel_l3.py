from string import Template
import sys
with open("test_parallel_l3.sh", "r") as f:
	src = Template(f.read())
def substition_vals(short_form:str, packet_rate: int,interrupt_interval: int,pci:int,trial:int,nics:int,alt:int=10,mode:int=1):
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
			"nics": nics,
			"trial": trial,
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
			"nics": nics,
			"trial": trial,
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
			"nics": nics,
			"trial": trial,
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
			"nics": nics,
			"trial": trial,
			"alt": alt,
			"mode":mode,
			"mode_type": mode_type
			
			}
	return {}
start_line = "#!/bin/bash\n"

packet_rates = [5,10,15,20,25,30]
interrupt_intervals = [5,10,20]
trials = [10]
num_nics = [1,2,4,8]
alterations = [10]
modes = [1]#,2,3]
job_list = []
#HERE
#this pass is to find the 100% load.
for trial in trials:
	for num in num_nics:
		for packet_rate in packet_rates:
			job_list.append(("T",packet_rate,0,1,trial,num))
			job_list.append(("T",packet_rate,0,5,trial,num))
			#for interrupt_interval in interrupt_intervals:
			#	job_list.append(("T",packet_rate,interrupt_interval,0,trial,num))
			#	job_list.append(("A",packet_rate,interrupt_interval,3,trial,num))
			#	job_list.append(("F",packet_rate,interrupt_interval,4,trial,num))
		#job_list.append(("T",9,1000,0))
#job_list.append(("T",15,0,1))
directory = 0
tests = [start_line] * 96
while len(job_list):
	j = job_list.pop()
	tests[directory] += src.substitute(substition_vals(j[0],j[1],j[2],j[3],j[4],j[5]))+"\n"
	directory += 1
	if directory == 96:
		directory = 0
#interrupt_intervals = [1000,2000,3000,4000,5000,6000,7000]
#packet_rates = [3,6,9,12,15,18]
#for packet_rate in packet_rates:
#	for interrupt_interval in interrupt_intervals:
#		job_list.append(("F",packet_rate,interrupt_interval,0))
#		job_list.append(("D",packet_rate,interrupt_interval,0))
#		job_list.append(("T",packet_rate,interrupt_interval,0))
#while len(job_list):
#	j = job_list.pop()
#	tests[directory] += src.substitute(substition_vals(j[0],j[1],j[2],j[3]))+"\n"
#	directory += 1
#	if directory == 10:
#		directory = 0

for i in range(0,96):
	with open("test_parallel"+str(i)+".sh", "w") as f:
		f.write(tests[i])
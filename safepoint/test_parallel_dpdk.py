from string import Template
import sys
with open("test_parallel_dpdk.sh", "r") as f:
	src = Template(f.read())
def substition_vals(short_form:str, packet_rate: int,interrupt_interval: int,pci:int,alt:int=10,mode:int=1):
	if pci ==1:
		pci_type = "PCI"
	elif pci == 2:
		pci_type = "SIG"
	elif pci == 0:
		pci_type = "TIMER"
	elif pci == 3:
		pci_type = "APIC"
	elif pci == 4:
		pci_type = "UIPI"
	elif pci == 5:
		pci_type = "SPIN"
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
	if short_form == "A":
		return {
			"int_type": "Apic",
			"packet_rate": packet_rate,
			"interrupt_interval": interrupt_interval,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "Apic",
			"alt": alt,
			"mode":mode,
			"mode_type": mode_type
			
			}
	return {}
start_line = "#!/bin/bash\n"

packet_rates = [i*0.25 for i in range(1,5)]+[i*0.5 for i in range(3,9)]+[i*0.25 for i in range(18,22)]
interrupt_intervals = [1,5,15]
alterations = [10]
modes = [1]#,2,3]
job_list = []
#HERE
for packet_rate in packet_rates:
	#job_list.append(("T",packet_rate,0,1,alt,mode))
	for interrupt_interval in interrupt_intervals:
		#job_list.append(("T",packet_rate,interrupt_interval,0,alt,mode))
		job_list.append(("T",packet_rate,interrupt_interval,0))
		job_list.append(("F",packet_rate,interrupt_interval,4))
		job_list.append(("A",packet_rate,interrupt_interval,3))
		job_list.append(("T",packet_rate,interrupt_interval,1))
		
	job_list.append(("T",packet_rate,0,5))
			#	job_list.append(("T",packet_rate,interrupt_interval,0,trial,num))
#job_list.append(("T",9,1000,0))
#job_list.append(("T",15,0,1))
directory = 0
tests = [start_line] * len(job_list)
while len(job_list):
	j = job_list.pop()
	tests[directory] += src.substitute(substition_vals(j[0],j[1],j[2],j[3]))+"\n"
	directory += 1
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

for i in range(0,len(tests)):
	with open("test_parallel"+str(i)+".sh", "w") as f:
		f.write(tests[i])
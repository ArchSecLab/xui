from string import Template
import sys
with open("test_parallel_chase.sh", "r") as f:
	src = Template(f.read())
def substition_vals(short_form:str, time_delta: int,size_in: int):
	if short_form == 'N':
		return {
			"int_type": "N",
			"size_in": size_in,
			"time_delta": time_delta,
			"int_type_full": "NoInterrupt",
			
			}
	if short_form == "F":
		return {
			"int_type": "Flush",
			"size_in": size_in,
			"time_delta": time_delta,
			"int_type_full": "Flush",
			}
	if short_form == "D":
		return {
			"int_type": "Drain",
			"size_in": size_in,
			"time_delta": time_delta,
			"int_type_full": "Drain",
			
			}
	return {}
start_line = "#!/bin/bash\n"
interrupt_intervals = [10000]
sizes = [1,2,3,4,5,6,7,8]
job_list = []
#HERE
for size in sizes:
	#job_list.append(("T",packet_rate,0,1,alt,mode))
	for interrupt_interval in interrupt_intervals:
		#job_list.append(("T",packet_rate,interrupt_interval,0,alt,mode))
		job_list.append(("F",interrupt_interval,size))
		job_list.append(("D",interrupt_interval,size))
#job_list.append(("T",9,1000,0))
#job_list.append(("T",15,0,1))
directory = 0
tests = [start_line] * len(job_list)
while len(job_list):
	j = job_list.pop()
	tests[directory] += src.substitute(substition_vals(j[0],j[1],j[2]))+"\n"
	directory += 1
	#if directory == 96:
	#	directory = 0
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

for i in range(len(tests)):
	with open("test_parallel"+str(i)+".sh", "w") as f:
		f.write(tests[i])
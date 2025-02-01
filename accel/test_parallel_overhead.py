from string import Template
import sys
with open("test_parallel_overhead.sh", "r") as f:
	src = Template(f.read())
def substition_vals(short_form:str, interrupt_interval: int,is_hard_timer:int,test_name:str):
	if short_form == 'N':
		return {
			"int_type": "N",
			"interrupt_interval": interrupt_interval,
			"int_type_full": "NoInterrupt",
			"is_hard_timer": is_hard_timer,
			"test_name": test_name
			}
	if short_form == "F":
		return {
			"int_type": "Flush",
			"interrupt_interval": interrupt_interval,
			"int_type_full": "Flush",
			"is_hard_timer": is_hard_timer,
			"test_name": test_name
			}
	if short_form == "D":
		return {
			"int_type": "Drain",
			"interrupt_interval": interrupt_interval,
			"int_type_full": "Drain",
			"is_hard_timer": is_hard_timer,
			"test_name": test_name
			}
	if short_form == "T":
		return {
			"int_type": "Track",
			"interrupt_interval": interrupt_interval,
			"int_type_full": "Track",
			"is_hard_timer": is_hard_timer,
			"test_name": test_name
			
			}
	return {}
start_line = "#!/bin/bash\n"

interrupt_intervals = [1,3,5,10,20,50,100]
test_names = ["chase"]
is_hard_timer = [0,1]
job_list = []
#HERE
#this pass is to find the 100% load.
for test_name in test_names:
	job_list.append(("T",140000000,1,test_name))
	for interrupt_interval in interrupt_intervals:
		job_list.append(("F",interrupt_interval,0,test_name))
		job_list.append(("D",interrupt_interval,0,test_name))
		job_list.append(("T",interrupt_interval,0,test_name))
		job_list.append(("T",interrupt_interval,1,test_name))
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

for i in range(len(tests)):
	with open("test_parallel"+str(i)+".sh", "w") as f:
		f.write(tests[i])
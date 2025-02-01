from string import Template
import sys
import subprocess
from math import sqrt
with open("test_parallel_accel.sh", "r") as f:
	src = Template(f.read())
def substition_vals(short_form:str, max_error: float,request_length: int,pci:int,vmr:float,stddev:float):
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
	if short_form == 'N':
		return {
			"int_type": "N",
			"request_length": request_length,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "NoInterrupt",
			"max_error": max_error,
			"vmr":vmr,
			"stddev":stddev
			}
	if short_form == "F":
		return {
			"int_type": "Flush",
			"request_length": request_length,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "Flush",
			"max_error": max_error,
			"vmr":vmr,
			"stddev":stddev
			}
	if short_form == "D":
		return {
			"int_type": "Drain",
			"request_length": request_length,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "Drain",
			"max_error": max_error,
			"vmr":vmr,
			"stddev":stddev
			
			}
	if short_form == "T":
		return {
			"int_type": "Track",
			"request_length": request_length,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "Track",
			"max_error": max_error,
			"vmr":vmr,
			"stddev":stddev
			
			}
	if short_form == "A":
		return {
			"int_type": "Apic",
			"request_length": request_length,
			"pci":pci,
			"pci_type":pci_type,
			"int_type_full": "Apic",
			"max_error": max_error,
			"vmr":vmr,
			"stddev":stddev
		}
	return {}
start_line = "#!/bin/bash\n"
request_lengths = [2,5,10,20,30,50,70,100]
max_errors = [1]
vmrs = [0.001,0.002,0.003,0.004,0.005,0.01,0.02,0.03]#,2,3]
stddev = []
for vmr in vmrs:
	stddev.append(round(sqrt(vmr*75.0),4))

job_list = []
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
def get_test_values(test):
	test_gist = test[len('done'):]
	us = test_gist.find('us')
	test_value_load = test_gist[:us]
	track = test_gist[us:].find('Track')
	apic = test_gist[us:].find('Apic')
	
	if track != -1:
		shorthand ='T'
		test_track_rem = test_gist[us:][track+len('Track'):]
		pci   = test_track_rem.find('PCI')
		spin  = test_track_rem.find('SPIN')
		timer = test_track_rem.find('TIMER')
		if pci != -1:
			num = 1
			maxerr = test_track_rem[pci+len('PCI'):]
			vmr = test_track_rem[:pci]
		elif spin != -1:
			num = 5
			maxerr = test_track_rem[spin+len('SPIN'):]
			vmr = test_track_rem[:spin]
		else:
			num = 0
			maxerr = test_track_rem[timer+len('TIMER'):]
			vmr = test_track_rem[:timer]
	elif apic != -1:
		shorthand ='A'
		test_apic_rem = test_gist[us:][apic+len('Apic'):]
		strat = test_apic_rem.find('APIC')
		num = 3
		maxerr = test_apic_rem[strat+len('APIC'):]
		vmr = test_apic_rem[:strat]
	return (shorthand, float(maxerr), int(test_value_load), num, int(vmr))
	
already_done = set()
#doneFiles = get_done_tests()
#for donefile in doneFiles:
#	entry = get_test_values(donefile)
#	already_done.add(entry)
cnt = 0
def jobber(job):
	global job_list, already_done, cnt
	if job not in already_done:
		job_list.append(job)
print(stddev)
#[0.2739, 0.3873, 0.4743, 0.5477, 0.6124, 0.866, 1.2247, 1.5]
#Lean(0) 4 30
#Busy(5) 0    20
#PCI(1)  0    10
#PCI(1)  1  70
#UIPI(4)  2 70
#PCI(1)  -1  70
#PCI (1) -1  100
# vmr = round(stddev[4]*stddev[4]/30,8) #0.6124*0.6124/30
# job_list.append(("T",1,30,0,vmr,stddev[4])) #Lean 4 30
# vmr = round(stddev[0]*stddev[0]/20,8) #0.2739*0.2739/20
# job_list.append(("T",1,20,5,vmr,stddev[0])) #Busy 0 20
# vmr = round(stddev[0]*stddev[0]/10,8) #0.2739*0.2739/10
# job_list.append(("T",1,10,1,vmr,stddev[0])) #PCI 0 10
# vmr = round(stddev[1]*stddev[1]/70,8) #0.3873*0.3873/70
# job_list.append(("T",1,70,1,vmr,stddev[1])) #PCI 1 70
vmr = round(stddev[2]*stddev[2]/70,8) #0.4743*0.4743/70
job_list.append(("F",1,70,4,vmr,stddev[2])) #UIPI 2 70
# vmr = round(stddev[-1]*stddev[-1]/70,8) #1.5*1.5/70
# job_list.append(("T",1,70,1,vmr,stddev[-1])) #PCI -1 70
# vmr = round(stddev[-1]*stddev[-1]/100,8) #1.5*1.5/100
# job_list.append(("T",1,100,1,vmr,stddev[-1])) #PCI -1 100
#for request_length in request_lengths:
#	for i in range(len(vmrs)):
#		vmr = round(stddev[i]*stddev[i]/request_length,8)
#		for max_error in max_errors:
#			job_list.append(("T",max_error,request_length,1,vmr,stddev[i]))
#			job_list.append(("T",max_error,request_length,0,vmr,stddev[i]))
#			job_list.append(("T",max_error,request_length,5,vmr,stddev[i]))
#			job_list.append(("A",max_error,request_length,3,vmr,stddev[i]))
#			job_list.append(("F",max_error,request_length,4,vmr,stddev[i]))
#print(job_list)
#print(already_done)
print(cnt)
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
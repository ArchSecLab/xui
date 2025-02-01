import os
import psutil
import time
import re

def find_and_kill_process_writing_to_file(file_path):
    for proc in psutil.process_iter(['pid', 'name', 'open_files']):
        try:
            open_files = proc.info['open_files']
            if open_files:
                for open_file in open_files:
                    if file_path in open_file.path:
                        create_time = proc.create_time()
                        current_time = time.time()
                        if (current_time - create_time) >  600:  # 600 seconds = 10 minutes
                            print(f"Killing process {proc.info['name']} with PID {proc.info['pid']} writing to {file_path}")
                            proc.kill()
                            return
        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue
good_files = {}
def scan_for_stalls():
    global good_files
    # equivalent to `ps aux | grep test`
    for proc in psutil.process_iter(['pid', 'name', 'create_time']):
        if "test_parallel" not in proc.info['name'] and "bash" not in proc.info['name']: # in the form test_parallel{number}.sh
            continue
        try:
            match = re.match(r"test_parallel(\d+)\.sh", proc.info['name'])
            if match:
                number = match.group(1)
                print(f"Found process with number: {number}")
                if number in good_files.keys():
                    continue
                #iterate over outschase/out{number}_* files
                early_exit = False
                for i in range(40):
                    if early_exit:
                        break
                    check_path = f"outsoverhead/out{number}_{i}.txt"
                    if os.path.exists(check_path):
                        #in this file we want to see if there is a line that says "number_print_from_gem5: 500"
                        with open(check_path, "r") as f:
                            lines = f.readlines()
                            is_good = False
                            for line in lines:
                                if f"{number}_print_from_gem5: 500" in line:
                                    is_good = True
                                    good_files[number] = True
                                    early_exit = True
                                    break
                            if not is_good:
                                # pgrep -P {proc.info['pid']} | xargs kill -9
                                print(f"Found bad file: {check_path}")
                                pgreps = os.popen(f"pgrep -P {proc.info['pid']}").read().split("\n")
                                for pid in pgreps:
                                    #we want to see if it has been running for 5 minutes
                                    if pid:
                                        create_time = psutil.Process(int(pid)).create_time()
                                        current_time = time.time()
                                        if (current_time - create_time) > 10000:
                                            print(f"Killing process {proc.info['name']} with PID {proc.info['pid']}")
                                            #os.system(f"kill -9 {proc.info['pid']}")
                                break

        except (psutil.NoSuchProcess, psutil.AccessDenied):
            continue

def scan_for_stalls_l3():
    for i in range(40):
        latency_path = os.path.join(directory, f"dir{i}/latency.txt")
        if os.path.exists(latency_path) and os.path.getsize(latency_path) == 0:
            print(f"Found empty file: {latency_path}")
            find_and_kill_process_writing_to_file(f"dir{i}/accel_latency")

def scan_periodically(directory):
    max_runtime = 0
    max_runtime_old = 0
    max_runtime_pid = -1
    while True:
        max_runtime_old = max_runtime
        killing = False
        for proc in psutil.process_iter(['pid', 'name', 'create_time']):
            if not proc.info['name'] == ('g'):
                continue
            try:
                create_time = proc.info['create_time']
                current_time = time.time()
                runtime = current_time - create_time
                if runtime > 600:  # 1800 seconds = 30 minutes
                    print(f"Process {proc.info['name']} with PID {proc.info['pid']} has been running for more than 30 minutes")
                    killing = True
                if runtime > 10000: # 18000 seconds = 5 hours
                    print(f"Process {proc.info['name']} with PID {proc.info['pid']} has been running for more than 5 hours")
                    proc.kill()
                if runtime > max_runtime:
                    max_runtime = runtime
                    max_runtime_pid = proc.info['pid']
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                continue
        if max_runtime > max_runtime_old:
            print(f"New maximum runtime: {max_runtime:.2f} seconds for PID {max_runtime_pid}")

        if killing:
            scan_for_stalls_l3()
        time.sleep(600)
if __name__ == "__main__":
    scan_periodically("paralel")

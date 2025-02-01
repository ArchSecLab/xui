import os
import psutil
import time

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

def scan_for_stalls(directory):
    for i in range(40):
        latency_path = os.path.join(directory, f"dir{i}/latency.txt")
        if os.path.exists(latency_path) and os.path.getsize(latency_path) == 0:
            print(f"Found empty file: {latency_path}")
            find_and_kill_process_writing_to_file(f"dir{i}/latency.txt")
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
                if runtime > 600:  # 600 seconds = 10 minutes
                    print(f"Process {proc.info['name']} with PID {proc.info['pid']} has been running for more than 10 minutes")
                    killing = True
                if runtime > max_runtime:
                    max_runtime = runtime
                    max_runtime_pid = proc.info['pid']
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                continue
        if max_runtime > max_runtime_old:
            print(f"New maximum runtime: {max_runtime:.2f} seconds for PID {max_runtime_pid}")
        if killing:
            scan_for_stalls(directory)
        time.sleep(300)
if __name__ == "__main__":
    scan_periodically("paralel")

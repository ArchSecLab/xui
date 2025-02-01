import os
import subprocess
import time
import signal
from threading import Thread, Lock

NUM_SCRIPTS = 80
SCRIPT_TEMPLATE = "test_parallel{}.sh"
PARALLEL_DIR_TEMPLATE = "paralel/dir{}"
MAX_RUNNABLE = 40

lock = Lock()
lock2 = Lock()
running_scripts = set()
completed_scripts = set()
next_script_index = 0
subprocesses = []
interrupted = False

start_times = [-1] * NUM_SCRIPTS
end_times = [-1] * NUM_SCRIPTS
pid = [-1] * NUM_SCRIPTS
script_dir = [-1] * NUM_SCRIPTS

def run_script(script_index, dir_index):
    global next_script_index, interrupted
    script_name = SCRIPT_TEMPLATE.format(script_index)
    dir_name = PARALLEL_DIR_TEMPLATE.format(dir_index)
    script_dir[script_index] = dir_index
    with lock2:
        if interrupted:
            return
        os.makedirs(dir_name, exist_ok=True)
        script_path = os.path.join(dir_name, script_name)
        subprocess.run(["cp", script_name, script_path])
        start_times[script_index] = time.time()
        os.chdir(dir_name)
        with open(script_name, "r") as script_file:
            lines = script_file.readlines()
            if len(lines) > 1:
                second_line = lines[1].strip()
                first_command_args = second_line.split()
                type_of_interrupt = first_command_args[3]
                interval = first_command_args[5]
                is_hard_timer = first_command_args[7]
                test_name = first_command_args[9]
        output_file = os.path.join("..", "..", "outs", f"out{type_of_interrupt}{interval}_hard{is_hard_timer}_{test_name}")
        with open(output_file, "w") as out:
            proc = subprocess.Popen(["bash", script_name], stdout=out, stderr=out)
        os.chdir("../..")
        subprocesses.append(proc)
    proc.wait()
    end_times[script_index] = time.time()
    run_duration = end_times[script_index] - start_times[script_index]
    print(f"Script {script_index} in directory {dir_name} ran for {run_duration:.2f} seconds")
    with lock2:
        if interrupted:
            return
    with lock:
        running_scripts.remove(script_index)
        completed_scripts.add(script_index)
        if next_script_index < NUM_SCRIPTS:
            new_script_index = next_script_index
            next_script_index += 1
            running_scripts.add(new_script_index)
            Thread(target=run_script, args=(new_script_index, dir_index)).start()

def signal_handler(sig, frame):
    global interrupted
    print("Interrupt received, terminating all subprocesses...")
    with lock2:
        interrupted = True
        for proc in subprocesses:
            for child in psutil.Process(proc.pid).children(recursive=True):
                child.terminate()
            proc.terminate()
        for proc in subprocesses:
            proc.wait()
        print("All subprocesses terminated.")

        exit(0)

def main():
    global next_script_index
    signal.signal(signal.SIGINT, signal_handler)
    next_script_index = MAX_RUNNABLE
    for i in range(0, min(MAX_RUNNABLE, NUM_SCRIPTS)):
        running_scripts.add(i)
        Thread(target=run_script, args=(i, i)).start()

    while len(completed_scripts) < NUM_SCRIPTS:
        time.sleep(120)
        for i in range(NUM_SCRIPTS):
            if start_times[i] != -1 and end_times[i] == -1:
                running_for = time.time() - start_times[i]
                print(f"Script {i} has been running for {running_for:.2f} seconds, at {script_dir[i]}")
    # delete all the scripts
    for i in range(0, NUM_SCRIPTS):
        script_name = SCRIPT_TEMPLATE.format(i)
        os.remove(script_name)

if __name__ == "__main__":
    main()

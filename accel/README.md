# Instructions to run accelerator experiments (Figure 9)

## Accelerator I/O notification experiment
This experiment is trying to evaluate the latency and polling cost of accelerator requests.
The considered case is where a thread waits for an accelerator request to be completed,
the thread either polls the accelerator or yields the CPU to a lower priority thread, or a mix
of both where the higher priority accelerator requester thread will wake up
and try polling the accelerator after expected amount of time has passed. Noise is introduced to the accelerator reply
 latency to characterize the polling cost of timer-based polling. We use our gem5 user timer interrupts,
 and user device interrupts to perform this experiment.

## Build gem5 and extract the disk image

Compile gem5 using `scons`:
```
scons build/X86/gem5.opt -j 40
```
During compilation, press [Enter], then type y when prompted.

Run the following scripts to extract the necessary image for experiments:
```
cp <path-to-archive>/ubuntu-accel-overhead.tar.gz .
tar -xvzf ubuntu-accel-overhead.tar.gz
```

## Prepare the parallel workload

Run the following scripts to prepare the parallel workload:
```
./createdir-accel.sh
python3 test_parallel_accel.py
```

## Run experiments

Run the following experiment script, with the specified monitoring script. The experiment can take up to half a day or more to complete. **Therefore, please use tmux to run the script.**
```
python3 parallel_manager.py
```
In another tmux window, run the following monitoring script:
```
python3 scan_for_stalls.py
```
When the experiment script finishes, kill the monitoring script.

## Generate Figure 9

To generate Figure 9, run the following command:
```
python3 lat_accel.py
```
The result will be saved as `fig_accelerator_repr.pdf`.

# Instructions to run overhead experiments (Figure 5)

## Overhead experiment
This experiment characterizes the overhead of different interrupt models implemented in gem5.
We use one integer heavy, one memory heavy, and one floating point heavy workload to
evaluate the overhead of different interrupt models.

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
./createdir-overhead.sh
python3 test_parallel_overhead.py
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

## Generate Figure 5

To generate Figure 5, run the following command:
```
python3 plot_overhead.py
```
The result will be saved as `fig_overhead_vs_interval.pdf`.

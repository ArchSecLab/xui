# Instructions to run l3 experiments (Figure 8)

## L3 Forwarding experiment
This experiment utilizes the L3 forwarding program from the DPDK library. We utilize our gem5-based
NIC implementation adapted from (Enabling Kernel Bypass Networking on gem5)[https://arxiv.org/pdf/2301.09470]
and our device user interrupt mechanism to contrast the wastage of cycles due to polling versus a low-latency
interrupt-based approach.

## Build gem5 and extract the disk image

Compile gem5 using `scons`:
```
scons build/X86/gem5.opt -j 40
```
During compilation, press [Enter], then type y when prompted.

Run the following scripts to extract the necessary image for experiments:
```
cp <path-to-archive>/ubuntu-l3.tar.gz .
tar -xvzf ubuntu-l3.tar.gz
```

## Prepare the parallel workload

Run the following scripts to prepare the parallel workload:
```
./createdir-l3.sh
python3 test_parallel_l3.py
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

## Generate Figure 8

To generate Figure 8, run the following command:
```
python3 find_cycles.py
```
The result will be saved as `fig-l3-forwarding.pdf`.

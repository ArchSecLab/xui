# Instructions to run safepoint experiments (Figure 6)

## Build gem5 and benchmark programs

Compile gem5 using `scons`:

```
scons build/X86/gem5.fast -j 24
```
During compilation, press [Enter], then type y when prompted.


Make sure you have the image archive from the box:

```
cp <path-to-archive>/ubuntu7.tar .
```

Run the following scripts to create the necessary image for experiments:

```
./one_time_setup_safepoint.sh
./make_disk_safepoint.sh
```

## Run experiments

Run the following experiment script. This process can take 1 to 3 hours to complete, depending on hardware. **Therefore, please use tmux to run the script.**
The results will be stored in the `result/safepoint` directory.

```
./run_safepoint.sh
```

The generated figure will be saved as `result/safepoint/figure6.pdf`

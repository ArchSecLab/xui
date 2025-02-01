# Instructions to run safepoint experiments (Figure 6)

## Build gem5 and benchmark programs
Pull the required submodules.

```
git submodule update --init -- aspen-gem5
```
If you have previously pulled the submodule and made modifications, the submodule update command may abort. In that case, consider renaming or backing up the existing `aspen-gem5` directory before running the update again.


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

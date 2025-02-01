# Instructions to run rocksdb experiments (Figure 7)

## Build gem5 and RocksDB

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
./one_time_setup_rocksdb.sh
./make_disk_rocksdb.sh
```

## Run experiments

Run the following experiment script. This process can take up to one day or more to complete. **Therefore, please use tmux to run the script.** The results will be stored in the `result/rocksdb` directory.
```
./run_rocksdb.sh
```



Once the terminal displays:  
> *"It might be ready to run 'python3 plot_rocksdb.py'"*  

You can generate Figure 7 by running the following command:
```
python3 plot_rocksdb.py
```
The generated figure will be saved as `result/rocksdb/figure7.pdf`

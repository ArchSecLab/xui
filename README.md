# xUI Artifacts
## How to operate
There are 7 experiments in this repo each of them have a folder that explains how to prepare and run the experiments.

As we use gem5 simulator with custom ubuntu images that have modified versions of programs like caladan and dpdk, we provide a box link for them:
* [rocksdb+safepoint base image](https://app.box.com/s/8rg9a709rezwmrak1d01elwaui3x89hd)
* [l3 image](https://app.box.com/s/k8h6q1ymdilcmyuqwfftgi1l6rj49sen)
* [accelerator+overhead image](https://app.box.com/s/9fxa7my5wnjkfy5swi8v2x7b0m8y1nko)

Here is how you can download the archives:
```
curl -L https://app.box.com/shared/static/9fxa7my5wnjkfy5swi8v2x7b0m8y1nko.gz --output ubuntu-accel-overhead.tar.gz
```
```
curl -L https://app.box.com/shared/static/k8h6q1ymdilcmyuqwfftgi1l6rj49sen.gz --output ubuntu-l3.tar.gz
```
```
curl -L https://app.box.com/shared/static/8rg9a709rezwmrak1d01elwaui3x89hd.tar --output ubuntu7.tar
```

There are also 2 experiments that involve a specific cpu feature (Intel User Interrupts), you might get unexpected results if you have a different cpu, as such we suggest the evaluator use our servers.

The gem5 simulations can take a long time, if you are pressed on time please let us know so that we can arrange a machine for speeding up the process.

Unfortunately our experiments cannot be further parallelized on a single machine, and starting more than one experiment at a time hurts run-time more than it helps.
We suggest running one tmux instance shared by all evaluators, to make sure no two experiments are running at the same time.

You can check if other experiments are running by:
```bash
ps aux | grep gem5
```
and:
```bash
ps aux | grep ./g
```

The timer, and profile experiments are the fastest running ones, we suggest finishing them first. The longest running experiment is the rocksdb experiment, expected to take 1.5 to 2 days. Accelerator experiments take around 10 hours. Overhead, safepoint, and l3 experiments each take 4-5 hours

Please run this in the current directory:
```mkdir mnt```

Please make sure you also have "result" directory in each folder.

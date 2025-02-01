#!/bin/bash
VAR=$((`nproc`))

for i in {0..31}
do
mkdir -p paralel/dir$i
cp configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor-dpdk.py paralel/dir$i/x86-ubuntu-run-with-kvm-uipi-template-processor-dpdk.py
done

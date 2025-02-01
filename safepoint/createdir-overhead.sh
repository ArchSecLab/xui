#!/bin/bash
VAR=$((`nproc`))
for i in {1..1}
do
   echo "Welcome $i times"
done

for i in {0..39}
do
mkdir -p paralel/dir$i
rm -f paralel/dir$i/g2
ln -s $PWD/build/X86/gem5.opt $PWD/paralel/dir$i/g2
cp configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor-overhead.py paralel/dir$i/x86-ubuntu-run-with-kvm-uipi-template-processor-overhead.py
mkdir -p paralel/dir$i/resources
cp resources/vmlinux paralel/dir$i/resources/vmlinux
cp configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-overhead.py.template paralel/dir$i/x86-ubuntu-run-with-kvm-uipi-overhead.py.template
# cp test_parallel$i.sh paralel/dir$i/test_parallel$i.sh
done


#!/bin/bash
VAR=$((`nproc`))
for i in {1..1}
do
   echo "Welcome $i times"
done

for i in {0..39}
do
mkdir -p paralel/dir$i
cp build/X86/gem5.opt paralel/dir$i/g
cp configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-template-processor-accel.py paralel/dir$i/x86-ubuntu-run-with-kvm-uipi-template-processor-accel.py
mkdir -p paralel/dir$i/resources
cp resources/vmlinux paralel/dir$i/resources/vmlinux
cp configs/example/gem5_library/x86-ubuntu-run-with-kvm-uipi-accel.py.template paralel/dir$i/x86-ubuntu-run-with-kvm-uipi-accel.py.template
#cp test_parallel$i.sh paralel/dir$i/test_parallel$i.sh
done

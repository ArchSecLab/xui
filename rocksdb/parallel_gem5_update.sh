#!/bin/bash
VAR=$((`nproc`))

for i in {0..31}
do
mkdir -p paralel/dir$i
cp build/X86/gem5.fast paralel/dir$i/g
cp build/X86/gem5.debug paralel/dir$i/gd

done

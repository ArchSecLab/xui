#!/bin/bash
cd util/m5
scons build/x86/out/m5
cp build/x86/out/libm5.a ../../aspen-gem5/safepoint-benchmarks/m5/libm5.a
cd ../../
cd util/term
make

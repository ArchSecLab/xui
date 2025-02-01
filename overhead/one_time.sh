#!/bin/bash
cd util/m5
# cp ../../uintr-measure/micro/uintr-ipc-bench/source/m5/src/SConscript src/SConscript
scons build/x86/out/m5
cp build/x86/out/libm5.a ../../uintr-measure/micro/programs/.
cd ../../

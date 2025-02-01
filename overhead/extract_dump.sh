if [ -z $1 ]
then
	diskName="ubuntu-x86.img"
else
	diskName="ubuntu-send-latest.img"
fi
util/gem5img.py mount $diskName ../mnt
cp ../mnt/usr/local/bin/dpdk-testpmd simple #uintr-measure/micro/programs/tests/429.mcf/src/mcf simple #uintr-ipc-bench/build/source/uintrfd/threaded simple
objdump -d simple > $diskName"asimple.x"
util/gem5img.py umount ../mnt

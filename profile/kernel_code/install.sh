# Install driver, alternative method
make clean
make

mknod /dev/uittmon c 250 0
chmod 666 /dev/uittmon
#instead of insmod:
#insmod uittmon.ko dyndbg
KERNELDIR=/lib/modules/`uname -r`
mkdir $KERNELDIR/extra
cp uittmon.ko $KERNELDIR/extra
depmod -ae
modprobe uittmon dyndbg==pmf

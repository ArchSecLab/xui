# Install driver, alternative method
make clean
make

mknod /dev/kvm_vmcs c 251 0
chmod 666 /dev/kvm_vmcs
#instead of insmod:
#insmod uittmon.ko dyndbg
KERNELDIR=/lib/modules/`uname -r`
mkdir $KERNELDIR/extra
cp kvm_vmcs.ko $KERNELDIR/extra
depmod -ae
modprobe kvm_vmcs dyndbg==pmf

# Make uittmon driver for Linux
# Last modified: 2020-08-17 Agner Fog
# See https://www.kernel.org/doc/html/latest/kbuild/modules.html


KERNELDIR := /lib/modules/`uname -r`/build
obj-m := kvm_vmcs.o
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
    
#$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules    

clean:
	rm -f .kvm_vmcs.*
	rm -f -r .tmp_versions
	rm -f *~
	rm -f kvm_vmcs.ko
	rm -f kvm_vmcs.o
	rm -f kvm_vmcs.mod.*
	rm -f linux.mdl
	rm -f Modules.symvers
    
install:
	./install.sh
    

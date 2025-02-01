# Make uittmon driver for Linux
# Last modified: 2020-08-17 Agner Fog
# See https://www.kernel.org/doc/html/latest/kbuild/modules.html


KERNELDIR := /lib/modules/`uname -r`/build
obj-m := uittmon.o
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
    
#$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules    

clean:
	rm -f .uittmon.*
	rm -f -r .tmp_versions
	rm -f *~
	rm -f uittmon.ko
	rm -f uittmon.o
	rm -f uittmon.mod.*
	rm -f linux.mdl
	rm -f Modules.symvers
    
install:
	./install.sh
    

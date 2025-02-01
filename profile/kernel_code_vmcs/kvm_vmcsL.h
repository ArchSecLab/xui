//                       msrdriver.h                    � 2015-12-22 Agner Fog

// Device driver for access to Model-specific registers and control registers
// in Linux (32 and 64 bit x86 platform)

// � 2010 - 2015 GNU General Public License www.gnu.org/licences

#ifndef KVM_VMCS_H
#define KVM_VMCS_H

#include "kvm_vmcs.h"

// #define DEV_MAJOR 222
#define DEV_MAJOR 251 // range 240-254 is vacant
#define DEV_MINOR 0
#define DEV_NAME "kvm_vmcs"

#define IOCTL_NOACTION _IO(DEV_MAJOR, 0)
#define IOCTL_MODIFY_VMCS _IO(DEV_MAJOR, 1)

#endif

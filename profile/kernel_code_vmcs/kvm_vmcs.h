//                       msrdriver.h                     2012-03-02 Agner Fog

// Device driver for access to Model-specific registers and control registers
// in Windows 2000 and later and Linux (32 and 64 bit x86 platform)

// (c) Copyright 2005-2012 by Agner Fog. GNU General Public License www.gnu.org/licences
#ifdef __KERNEL__
#include "linux/types.h"
#else
#include <stdint.h>
#endif

#pragma once

// input/output data structure for MSR driver
struct kvm_vmcs_io
{
#ifdef __KERNEL__
    u64 *sp;
#else
    uint64_t *sp;
#endif
    int receiver_pid;
    int uipi_index;
    int uintr_fd;
};

//                       msrdriver.h                    � 2015-12-22 Agner Fog

// Device driver for access to Model-specific registers and control registers
// in Linux (32 and 64 bit x86 platform)

// � 2010 - 2015 GNU General Public License www.gnu.org/licences

#ifndef UITTMON_H
#define UITTMON_H

#include "uittmon.h"

// #define DEV_MAJOR 222
#define DEV_MAJOR 250 // range 240-254 is vacant
#define DEV_MINOR 0
#define DEV_NAME "uittmon"

#define IOCTL_NOACTION _IO(DEV_MAJOR, 0)
#define IOCTL_PROCESS_LIST _IO(DEV_MAJOR, 1)
#define IOCTL_SET_PID _IO(DEV_MAJOR, 2)
#define IOCTL_GET_OFFSET _IO(DEV_MAJOR, 3)
#define IOCTL_SENDIPI _IO(DEV_MAJOR, 4)
#define IOCTL_REGISTER_HANDLER _IO(DEV_MAJOR, 5)
#define IOCTL_REGISTER_STATE _IO(DEV_MAJOR, 6)

#endif

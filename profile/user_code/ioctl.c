#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "uittmonL.h"

int main() {
    int fd = open("/dev/uittmon", O_RDWR);
    if (fd < 0) {
        perror("Failed to open /dev/uittmon");
        return EXIT_FAILURE;
    }

    if (ioctl(fd, IOCTL_NOACTION) < 0) {
        perror("IOCTL_NOACTION failed");
        //close(fd);
        //return EXIT_FAILURE;
    }
    printf("IOCTL_NOACTION succeeded\n");

    uint64_t bogus = 0;
    struct uittmon_io arg = {0};
    printf("%llu\n", &arg);
    arg.receiver_pid = getpid();
    printf("pid: %llu\n", getpid());
    //arg.sp = &bogus;

    if (ioctl(fd, IOCTL_PROCESS_LIST, &arg) < 0) {
        perror("IOCTL_PROCESS_LIST failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("IOCTL_PROCESS_LIST succeeded\n");

    close(fd);
    return EXIT_SUCCESS;
}

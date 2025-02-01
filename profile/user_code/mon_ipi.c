#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
// #include <x86gprintrin.h>

#include <errno.h>

#define __USE_GNU
#include <pthread.h>
#include <sched.h>

#include "../kernel_code/uittmonL.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#ifndef __NR_uintr_register_handler
#define __NR_uintr_register_handler 471
#define __NR_uintr_unregister_handler 472
#define __NR_uintr_create_fd 473
#define __NR_uintr_register_sender 474
#define __NR_uintr_unregister_sender 475
#define __NR_uintr_wait 476
#endif
#define uintr_register_handler(handler, flags) syscall(__NR_uintr_register_handler, handler, flags)
#define uintr_unregister_handler(flags) syscall(__NR_uintr_unregister_handler, flags)
#define uintr_create_fd(vector, flags) syscall(__NR_uintr_create_fd, vector, flags)
#define uintr_register_sender(fd, flags) syscall(__NR_uintr_register_sender, fd, flags)
#define uintr_unregister_sender(ipi_idx, flags) syscall(__NR_uintr_unregister_sender, ipi_idx, flags)
#define uintr_wait(flags) syscall(__NR_uintr_wait, flags)
#define NUM_TRIES 40000000000
// 00
// 00 //000
volatile uint64_t *sp = NULL;
volatile int uintr_fd, uipi_index;
pid_t receiver_pid;

volatile int received = 0;

volatile int driver_fd = -1;

volatile int initialized = 0;

enum __attribute__((__packed__)) InterruptState
{
  INLOOP,
  RETURNFROMINTERRUPT
};
volatile enum InterruptState interrupt_state = INLOOP;

uint64_t t, tuint;
uint64_t t_interrupt_length;
uint64_t t0, t1, t2, t3, t4, t5, t6, t7;

struct uintr_upid
{
  struct
  {
    volatile uint8_t status;    // bit 0: ON, bit 1: SN, bit 2-7: reserved
    volatile uint8_t reserved1; // Reserved
    volatile uint8_t nv;        // Notification vector
    volatile uint8_t reserved2; // Reserved
    volatile uint32_t ndst;     // Notification destination
  } nc;                         // Notification control
  volatile uint64_t puir;       // Posted user interrupt requests
};

struct uintr_upid *receiver_upid;

void driver_give_info()
{
  printf("pid: %d\n", getpid());
  if (ioctl(driver_fd, IOCTL_SET_PID, getpid()) < 0)
  {
    printf("Huh?\n");
    perror("IOCTL_SET_PID failed");
    close(driver_fd);
    exit(1);
  }
  initialized = 1;
}

void __attribute__((interrupt))
__attribute__((target("general-regs-only", "inline-all-stringops")))
ui_handler(struct __uintr_frame *ui_frame,
           unsigned long long vector)
{
  uint64_t tstart = _rdtsc();
  interrupt_state = RETURNFROMINTERRUPT;
  t1 = t;
  if (t0 > t1)
  {
    t0 = t;
  }
  uint64_t tsc_till = _rdtsc() + 2 * 2000;
  while (_rdtsc() < tsc_till)
    ;
  t_interrupt_length = _rdtsc() - tstart;
}
static inline void *__movsb(void *d, const void *s, size_t n)
{
  asm volatile("rep movsb"
               : "=D"(d),
                 "=S"(s),
                 "=c"(n)
               : "0"(d),
                 "1"(s),
                 "2"(n)
               : "memory");
  return d;
}
void reciever()
{
  printf("Hello from recv\n");
  // Enable interrupts
  // driver_give_info();
  ioctl(driver_fd, IOCTL_REGISTER_STATE, &interrupt_state);
  _stui();
  uint64_t *buffer = malloc(sizeof(uint64_t) * NUM_TRIES);
  void *buffer2 = malloc(sizeof(uint64_t) * 1000);
  uint64_t i = 0;
  uint64_t t_temp;
  printf("receiver started\n");
  asm("mov %%rsp, %0" : "=rm"(sp));
  asm("sfence");
  t = _rdtsc();
  for (i = 0; i < NUM_TRIES; i++)
  {
    asm volatile("mov %%eax, %%eax" ::: "eax");
    asm volatile("mov %%eax, %%eax" ::: "eax");
    asm volatile("mov %%eax, %%eax" ::: "eax");
    asm volatile("mov %%eax, %%eax" ::: "eax");
    asm volatile("mov %%eax, %%eax" ::: "eax");
    asm volatile("mov %%eax, %%eax" ::: "eax");
  }
  received = 1;
  uint64_t sum = _rdtsc() - t;
  printf("Average: %lf\n", (double)sum);
}

/*void driver_invoke(){*/
/*  int fd = open("/dev/uittmon", O_RDWR);*/
/**/
/*  struct uittmon_io ioctl_args = {0};*/
/**/
/*  ioctl_args.sp = sp;*/
/*  ioctl_args.receiver_pid = receiver_pid;*/
/*  ioctl_args.uipi_index = uipi_index;*/
/*  ioctl_args.uintr_fd = uintr_fd;*/
/**/
/*  if (ioctl(fd, IOCTL_PROCESS_LIST, &ioctl_args) < 0) {*/
/*    perror("IOCTL_PROCESS_LIST failed");*/
/*    close(fd);*/
/*  }*/
/*  close(fd);*/
/*}*/

void sender_listener(void *arg)
{
  while (!sp)
  {
    asm("lfence");
  }
  uint64_t i = 0;
  uint64_t perc = 0;
  const uint64_t one_perc = NUM_TRIES / 100;
  while (!received)
  {
    while (interrupt_state != INLOOP)
      ;
    ioctl(driver_fd, IOCTL_SENDIPI, 0);
    //_senduipi(uipi_index);
    for (size_t j = 0; j < 20000; j++)
    {
      asm volatile("mov %%eax, %%eax" ::: "eax");
      asm volatile("mov %%eax, %%eax" ::: "eax");
      asm volatile("mov %%eax, %%eax" ::: "eax");
      asm volatile("mov %%eax, %%eax" ::: "eax");
      asm volatile("mov %%eax, %%eax" ::: "eax");
      asm volatile("mov %%eax, %%eax" ::: "eax");
    }

    i++;
    // if (i / one_perc > perc)
    // {
    //   perc++;
    //   printf("%%%d\n", perc);
    // }
  }
  printf("Sent %lu IPIs\n", i);
}

int main()
{
  driver_fd = open("/dev/uittmon", O_RDWR);
  printf("driver_fd: %d\n", driver_fd);
  int core1 = 20, core2 = 1;
  //  core2=0;
  // printf("Give me two sibling cores: <number> <number>\n");
  // scanf("%d %d ", core1,core2);
  // set_thread_affinity(core1);
  receiver_pid = getpid();

  // sender core2=0
  pthread_attr_t attrs;
  memset(&attrs, 0, sizeof(attrs));
  pthread_attr_init(&attrs);
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(core2, &set);
  pthread_attr_setaffinity_np(&attrs, sizeof(set), &set);
  pthread_t pt;

  // get receiver pid
  receiver_pid = getpid();
  // reciever core1=20
  CPU_ZERO(&set);
  CPU_SET(core1, &set);
  pthread_setaffinity_np(pthread_self(), sizeof(set), &set);

  if (pthread_create(&pt, &attrs, &sender_listener, NULL))
    exit(-1);
  reciever();
  pthread_join(pt, NULL);
  close(driver_fd);
}

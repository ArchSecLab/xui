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
#define NUM_TRIES 40000000
// 00
// 00 //000
volatile uint64_t *sp = NULL;
volatile int uintr_fd, uipi_index;
pid_t receiver_pid;

volatile int received = 0;

volatile int driver_fd = -1;

volatile int initialized = 0;

enum InterruptState
{
  INLOOP,
  RETURNFROMINTERRUPT
};
volatile enum InterruptState interrupt_state = INLOOP;

uint64_t t_1,t_2,t_3,t_4,t_5,t_6,t_7, tuint;
uint64_t t_interrupt_length;
uint64_t t0, t1, t2, t3, t4, t5, t6, t7;
uint64_t vector_in;
#ifdef SENDDIFF
uint64_t t_normal,t,t_short;
uint64_t send_normal_sum = 0, send_short_sum = 0;
#endif

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
  //printf("pid: %d\n", getpid());
  if (ioctl(driver_fd, IOCTL_SET_PID, getpid()) < 0)
  {
    printf("Huh?\n");
    perror("IOCTL_SET_PID failed");
    close(driver_fd);
    exit(1);
  }
  initialized = 1;
}
int count = 0;
int num; // Declare the variable num
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
void __attribute__((interrupt))
__attribute__((target("general-regs-only", "inline-all-stringops")))
ui_handler(struct __uintr_frame *ui_frame,
           unsigned long long vector)
{
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
  if (uintr_register_handler(ui_handler, 0))
    exit(-1);
  uintr_fd = uintr_create_fd(0, 0);
  if (uintr_fd < 0)
    exit(-1);
  // Enable interrupts
  driver_give_info();

  _stui();
  void *buffer = malloc(sizeof(uint64_t) * 1000);
  void *buffer2 = malloc(sizeof(uint64_t) * 1000);
  asm("mov %%rsp, %0" : "=rm"(sp));
  asm("sfence");
  while (!received)
  {
    // __movsb(buffer, buffer2, sizeof(uint64_t) * 1000);
    t = _rdtsc();
    t = _rdtsc();
    t = _rdtsc();
    t = _rdtsc();
    t = _rdtsc();
    t = _rdtsc();
     t = _rdtsc();
     t = _rdtsc();
     t = _rdtsc();
     t = _rdtsc();
     t = _rdtsc();
     t = _rdtsc();
    if (interrupt_state == RETURNFROMINTERRUPT)
    {
	  if(t_2 - t_1 > 100){
		t_7 = t_2;
	  }
	  else if(t_3 - t_2 > 100){
		t_7 = t_3;
	  }
	  else if(t_4 - t_3 > 100){
		t_7 = t_4;
	  }
	  else if(t_5 - t_4 > 100){
		t_7 = t_5;
	  }
	  else if(t_6 - t_5 > 100){
		t_7 = t_6;
	  }
	  else{
		t_7 = t_1;
	  }
      t7 = t_7 - t_interrupt_length;
      interrupt_state = INLOOP;
    }
  }
}

void map_upid()
{
  while (!initialized)
    ;
  uint64_t offset = 0;
  if (ioctl(driver_fd, IOCTL_GET_OFFSET, &offset) < 0)
  {
    perror("IOCTL_GET_OFFSET failed");
    exit(1);
  }


  receiver_upid = (struct uintr_upid *)((uint64_t)mmap(NULL, sizeof(struct uintr_upid), PROT_READ | PROT_WRITE, MAP_SHARED, driver_fd, 0) + (uint64_t)offset);
  if (receiver_upid == MAP_FAILED)
  {
    fprintf(stderr, "mmap failed: %s\n", strerror(errno));
    exit(1);
  }
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
  map_upid();
  uipi_index = uintr_register_sender(uintr_fd, 0);
  uint64_t sp_use = (uint64_t)sp - 128;
  uint64_t *temprsp_addr = (uint64_t *)(((uint64_t)sp_use & ~(uint64_t)0xf) - 8);
  uint64_t *uirrv_addr = (uint64_t *)(((uint64_t)sp_use & ~(uint64_t)0xf) - 32);
  uint64_t *handler_addr = (uint64_t *)(((uint64_t)sp_use & ~(uint64_t)0xf) - 40);
  uint64_t t0t1_sum = 0;
  uint64_t t1t2_sum = 0;
  uint64_t t2t3_sum = 0;
  uint64_t t3t4_sum = 0;
  uint64_t t4t5_sum = 0;
  uint64_t t5t6_sum = 0;
  uint64_t t6t7_sum = 0;
  uint64_t total_sum = 0;
  uint64_t *t0t1 = (uint64_t *)malloc(NUM_TRIES * 8 * sizeof(uint64_t));
  uint64_t *t1t2 = t0t1 + NUM_TRIES;
  uint64_t *t2t3 = t1t2 + NUM_TRIES;
  uint64_t *t3t4 = t2t3 + NUM_TRIES;
  uint64_t *t4t5 = t3t4 + NUM_TRIES;
  uint64_t *t5t6 = t4t5 + NUM_TRIES;
  uint64_t *t6t7 = t5t6 + NUM_TRIES;
  uint64_t *total = t6t7 + NUM_TRIES;
  uint64_t t0t1_max = 0;
  uint64_t t1t2_max = 0;
  uint64_t t2t3_max = 0;
  uint64_t t3t4_max = 0;
  uint64_t t4t5_max = 0;
  uint64_t t5t6_max = 0;
  uint64_t t6t7_max = 0;
  uint64_t total_max = 0;
  uint64_t t0t1_min = -1;
  uint64_t t1t2_min = -1;
  uint64_t t2t3_min = -1;
  uint64_t t3t4_min = -1;
  uint64_t t4t5_min = -1;
  uint64_t t5t6_min = -1;
  uint64_t t6t7_min = -1;
  uint64_t total_min = -1;
  uint64_t i = 0;
  uint64_t perc = 0;
  uint64_t inside = 0;
  uint64_t earlier_points = 0;
  uint64_t late_points = 0;
  const uint64_t one_perc = NUM_TRIES / 100;
  for (i = 0; i < NUM_TRIES; i++)
  {
    while (interrupt_state != INLOOP)
      ;
    inside = __rdtsc();
    while (inside + 3000 > __rdtsc())
      {
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
		}
	t_normal = _rdtsc();
    _senduipi(uipi_index);
	t_short = _rdtsc();
	send_normal_sum +=  t_short - t_normal;
    inside = __rdtsc();
  }

  for (i = 0; i < NUM_TRIES; i++)
  {
    while (interrupt_state != INLOOP)
      ;
    inside = __rdtsc();
    while (inside + 3000 > __rdtsc())
      {
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
asm volatile("nop");
		}
	int k = rand();
	t_normal = _rdtsc();
    __atomic_store_n(&(receiver_upid->nc.status), k&1, __ATOMIC_RELEASE);
    _senduipi(uipi_index);
	uint64_t send_start = _rdtsc();
	if(k & 1){	
		send_short_sum +=  (send_start - t_normal);
	}
	else{
		i--;} 
  }
  printf("Send without receive: %lf\n", (double)send_short_sum / (double)NUM_TRIES);
  printf("Send with receive: %lf\n", (double)send_normal_sum / (double)NUM_TRIES);
  received = 1;
  return;
}

int main()
{
  driver_fd = open("/dev/uittmon", O_RDWR);
  // printf("driver_fd: %d\n", driver_fd);
  int core1 = 10, core2 = 1;

  //  core2=0;
  // printf("Give me two sibling cores: <number> <number>\n");
  // scanf("%d %d ", core1,core2);
  // set_thread_affinity(core1);
  receiver_pid = getpid();
  char *num_str = getenv("NUM");
  num = 156;
  // bind sender to core 1
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

  // bind receiver to core 56 (sibling)
  CPU_ZERO(&set);
  CPU_SET(core1, &set);
  pthread_setaffinity_np(pthread_self(), sizeof(set), &set);

  if (pthread_create(&pt, &attrs, &sender_listener, NULL))
    exit(-1);
  reciever();
  pthread_join(pt, NULL);
  close(driver_fd);
}

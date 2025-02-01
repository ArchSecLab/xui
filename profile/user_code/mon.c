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
#define NUM_TRIES 400000
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
uint64_t t_normal,t_short;
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

#ifdef SENDMEASURE
  if (vector)
  {
#endif
    uint64_t tstart = _rdtsc();
	asm("lfence");
    interrupt_state = RETURNFROMINTERRUPT;
    vector_in += vector;
    count++;
    
    t0 = MAX(t_6,MAX(t_5,MAX(t_4,MAX(t_3,MAX(t_1,t_2)))));
	#ifdef SENDDIFF
	send_short_sum += t0 - t_normal;
	#endif
    // t1 = t;
    // if (t0 > t1)
    // {
    // t0 = t;
    // }
    uint64_t tsc_till = _rdtsc() + 2 * 2000;
    while (_rdtsc() < tsc_till)
      ;
    // __atomic_store_n(&(receiver_upid->puir), 0, __ATOMIC_SEQ_CST);
    t_interrupt_length = _rdtsc() - tstart;

#ifdef SENDMEASURE
  }
#endif
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
    t_1 = _rdtsc();
    t_2 = _rdtsc();
    t_3 = _rdtsc();
    t_4 = _rdtsc();
    t_5 = _rdtsc();
    t_6 = _rdtsc();
    // t = _rdtsc();
    // t = _rdtsc();
    // t = _rdtsc();
    // t = _rdtsc();
    // t = _rdtsc();
    // t = _rdtsc();
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
    while (inside + 2000 > __rdtsc())
      ;
#ifndef SENDDIFF
    __atomic_store_n(temprsp_addr, 0xDEADBEEF, __ATOMIC_RELAXED);
    __atomic_store_n(uirrv_addr, 0xDEADBEEF, __ATOMIC_RELAXED);
    __atomic_store_n(handler_addr, 0xDEADBEEF, __ATOMIC_RELAXED);
#endif
    inside = 0;
#ifdef SENDMEASURE
    int expected = 3;
    __atomic_store_n(&(receiver_upid->puir), 1 << 1, __ATOMIC_SEQ_CST);
#endif
#ifdef SENDDIFF
	t_normal = _rdtsc();
#endif
    _senduipi(uipi_index);
#ifdef SENDDIFF
	asm("lfence");
	uint64_t send_start = _rdtsc();
	send_normal_sum += - t_normal + send_start;
#endif
    // __atomic_store_n(&(receiver_upid->puir), 0, __ATOMIC_SEQ_CST);
#ifdef SENDMEASURE
    for (int j = 0; j < num; j++)
    {
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
      asm volatile("nop");
    }
      asm volatile("nop");
      asm volatile("nop");

    __atomic_compare_exchange_n(&(receiver_upid->puir), &expected, 1, 0, __ATOMIC_RELAXED, __ATOMIC_RELAXED);
    if (expected == 3)
    {
      earlier_points++;
    }
    else if (!expected)
    {
		if(earlier_points < late_points){
		i--;
    __atomic_store_n(&(receiver_upid->puir), 0, __ATOMIC_SEQ_CST);
    uint64_t tsc_till = _rdtsc() + 2 * 6000;
    while (_rdtsc() < tsc_till)
      ;
	continue;
		}
	else{
		
      late_points++;
	}
    }
#endif
    // __atomic_store_n(&(receiver_upid->puir), 1 << 2, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 3, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 4, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 5, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 6, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 7, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 8, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 9, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 10, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 11, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 12, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 13, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 14, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 15, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 16, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 17, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 18, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 19, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 20, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 21, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 22, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 23, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 24, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 25, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 26, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 27, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 28, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 29, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 30, __ATOMIC_SEQ_CST);
    // __atomic_store_n(&(receiver_upid->puir), 1 << 31, __ATOMIC_SEQ_CST);
//#ifdef SENDMEASURE
	asm("lfence");
    t1 = _rdtsc();
    while (__atomic_load_n(&(receiver_upid->nc.status), __ATOMIC_ACQUIRE) & 1)
     {
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		} 
    t2 = _rdtsc();

//#endif
    while (__atomic_load_n(&(receiver_upid->puir), __ATOMIC_ACQUIRE))
     {
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		} 
    t3 = _rdtsc();
    while (__atomic_load_n(temprsp_addr, __ATOMIC_ACQUIRE) == 0XDEADBEEF)
    {
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
    }
    t4 = _rdtsc();
    while (__atomic_load_n(uirrv_addr, __ATOMIC_ACQUIRE) == 0XDEADBEEF)
    {
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
    }
    t5 = _rdtsc();
    while (__atomic_load_n(handler_addr, __ATOMIC_ACQUIRE) == 0XDEADBEEF)
    {
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
		asm volatile("nop");
    }

    t6 = _rdtsc();
    __atomic_store_n(&(receiver_upid->puir), 0, __ATOMIC_SEQ_CST);
    uint64_t tsc_till = _rdtsc() + 2 * 6000;
    while (_rdtsc() < tsc_till)
      ;
    if (t7 < t6)
    {
      t7 = t6;
    }
    if (t0 > t1)
    {
      t0 = t1;
    }
    t0t1_sum += (t1 - t0);
    t1t2_sum += (t2 - t1);
    t2t3_sum += (t3 - t2);
    t3t4_sum += (t4 - t3);
    t4t5_sum += (t5 - t4);
    t5t6_sum += (t6 - t5);
    t6t7_sum += (t7 - t6);
    total_sum += (t7 - t0);
    t0t1[i] = (t1 - t0);
    t1t2[i] = (t2 - t1);
    t2t3[i] = (t3 - t2);
    t3t4[i] = (t4 - t3);
    t4t5[i] = (t5 - t4);
    t5t6[i] = (t6 - t5);
    t6t7[i] = (t7 - t6);
    total[i] = (t7 - t0);

    t0t1_max = (t0t1_max < t0t1[i] ? t0t1[i] : t0t1_max);
    t1t2_max = (t1t2_max < t1t2[i] ? t1t2[i] : t1t2_max);
    t2t3_max = (t2t3_max < t2t3[i] ? t2t3[i] : t2t3_max);
    t3t4_max = (t3t4_max < t3t4[i] ? t3t4[i] : t3t4_max);
    t4t5_max = (t4t5_max < t4t5[i] ? t4t5[i] : t4t5_max);
    t5t6_max = (t5t6_max < t5t6[i] ? t5t6[i] : t5t6_max);
    t6t7_max = (t6t7_max < t6t7[i] ? t6t7[i] : t6t7_max);
    total_max = (total_max < total[i] ? total[i] : total_max);

    t0t1_min = (t0t1_min > t0t1[i] ? t0t1[i] : t0t1_min);
    t1t2_min = (t1t2_min > t1t2[i] ? t1t2[i] : t1t2_min);
    t2t3_min = (t2t3_min > t2t3[i] ? t2t3[i] : t2t3_min);
    t3t4_min = (t3t4_min > t3t4[i] ? t3t4[i] : t3t4_min);
    t4t5_min = (t4t5_min > t4t5[i] ? t4t5[i] : t4t5_min);
    t5t6_min = (t5t6_min > t5t6[i] ? t5t6[i] : t5t6_min);
    t6t7_min = (t6t7_min > t6t7[i] ? t6t7[i] : t6t7_min);
    total_min = (total_min > total[i] ? total[i] : total_min);

    if (i / one_perc > perc)
    {
      perc++;
    }
  }
#ifdef SENDDIFF
  printf("Send without receive: %lf\n", (double)send_short_sum / (double)NUM_TRIES);
  printf("Send with receive: %lf\n", (double)send_normal_sum / (double)NUM_TRIES);
  received = 1;
  return;
#endif
#ifdef SENDMEASURE
  printf("FLUSH cycle count: %lf\n", (double)t0t1_sum / (double)NUM_TRIES);
  printf("Status change cycle count: %lf\n", (double)t1t2_sum / (double)NUM_TRIES);
  printf("Posted interrupt clear cycle count: %lf\n", (double)t2t3_sum / (double)NUM_TRIES);
#else
  printf("Delivery first stack push cycle count: %lf\n", (double)t3t4_sum / (double)NUM_TRIES);
  printf("Delivery vector push change cycle count: %lf\n", (double)t4t5_sum / (double)NUM_TRIES);
  printf("Handler push cycle count: %lf\n", (double)t5t6_sum / (double)NUM_TRIES);
  printf("uiret push cycle count: %lf\n", (double)t6t7_sum / (double)NUM_TRIES);
#endif
  printf("count: %d\n", count);
  printf("vector avg: %lf\n", (double)vector_in / (double)count);
  printf("earlier points: %d\n", earlier_points);
  printf("late points: %d\n", late_points);
 /* fprintf(stderr, "t0t1:\n");
  for (size_t i = 0; i < NUM_TRIES; i++)
  {
    fprintf(stderr, "%ld\n", t0t1[i]);
  }
  fprintf(stderr, "t1t2:\n");
  for (size_t i = 0; i < NUM_TRIES; i++)
  {
    fprintf(stderr, "%ld\n", t1t2[i]);
  }
  fprintf(stderr, "t2t3:\n");
  for (size_t i = 0; i < NUM_TRIES; i++)
  {
    fprintf(stderr, "%ld\n", t2t3[i]);
  }
  fprintf(stderr, "t3t4:\n");
  for (size_t i = 0; i < NUM_TRIES; i++)
  {
    fprintf(stderr, "%ld\n", t3t4[i]);
  }
  fprintf(stderr, "t4t5:\n");
  for (size_t i = 0; i < NUM_TRIES; i++)
  {
    fprintf(stderr, "%ld\n", t4t5[i]);
  }
  fprintf(stderr, "t5t6:\n");
  for (size_t i = 0; i < NUM_TRIES; i++)
  {
    fprintf(stderr, "%ld\n", t5t6[i]);
  }
  fprintf(stderr, "t6t7:\n");
  for (size_t i = 0; i < NUM_TRIES; i++)
  {
    fprintf(stderr, "%ld\n", t6t7[i]);
  }

  fprintf(stderr, "total:\n");
  for (size_t i = 0; i < NUM_TRIES; i++)
  {
    fprintf(stderr, "%ld\n", total[i]);
  }
*/
  received = 1;
}

int main()
{
  driver_fd = open("/dev/uittmon", O_RDWR);
  // printf("driver_fd: %d\n", driver_fd);
#ifndef SENDDIFF
  int core1 = 20, core2 = 0;
#else

  int core1 = 20, core2 = 0;
#endif

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

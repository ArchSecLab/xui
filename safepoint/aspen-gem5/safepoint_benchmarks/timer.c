
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
// #include <x86gprintrin.h>
#include <errno.h>

#include "bench.h"

#include <m5_mmap.h>

#define __USE_GNU
#include <pthread.h>
#include <sched.h>

#ifndef __NR_uintr_register_handler
#define __NR_uintr_register_handler	471
#define __NR_uintr_unregister_handler	472
#define __NR_uintr_create_fd		473
#define __NR_uintr_register_sender	474
#define __NR_uintr_unregister_sender	475
#define __NR_uintr_wait			476
#endif

#define uintr_register_handler(handler, flags)	syscall(__NR_uintr_register_handler, handler, flags)
#define uintr_unregister_handler(flags)		syscall(__NR_uintr_unregister_handler, flags)
#define uintr_create_fd(vector, flags)		syscall(__NR_uintr_create_fd, vector, flags)
#define uintr_register_sender(fd, flags)	syscall(__NR_uintr_register_sender, fd, flags)
#define uintr_unregister_sender(ipi_idx, flags)	syscall(__NR_uintr_unregister_sender, ipi_idx, flags)
#define uintr_wait(flags)			syscall(__NR_uintr_wait, flags)

#define TOKEN 0
// volatile long long preempt_cnt = 0;
long long preempt_cnt = 0, preempt_sent = 0;
int uintr_fd, uipi_index;
pthread_t tid;
volatile int preempt_start = 0;

//#ifdef CONCORD
__thread int concord_preempt_now;
volatile int *cpu_preempt_point;
// #endif 

long long timeslice_ns, preempt_overhead = 0;
long long start, end;

// long long now() {
// 	struct timespec ts;
// 	timespec_get(&ts, TIME_UTC);
// 	return ts.tv_sec * 1e9 + ts.tv_nsec;
// }

void reset_timer() {
	start = _rdtsc();
	preempt_cnt = 0;
}

void set_thread_affinity(int core) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);
	sched_setaffinity(0, sizeof(mask), &mask);
}

// #ifdef SIGNAL
void signal_handler(int signum) {
    ++preempt_cnt;
}
// #endif 

// void print_uintr() {printf("uintr~!~\n");}
// #ifdef UINTR
void __attribute__ ((interrupt))
     __attribute__((target("general-regs-only", "inline-all-stringops")))
     ui_handler(struct __uintr_frame *ui_frame,
		unsigned long long vector) {
    ++preempt_cnt;
	// print_uintr();
}
// #endif 

// #ifdef CONCORD
void concord_func() {
	// printf("yes\n");
    concord_preempt_now = 0;
    ++preempt_cnt;
}
// #endif

void *timer(void* __attribute__((unused)) arg) {
    set_thread_affinity(1);

// #ifdef SIGNAL
// 	// block signals for the timer thread
// 	sigset_t mask;
//     sigemptyset(&mask);
//     sigaddset(&mask, SIGUSR1);
//     pthread_sigmask(SIG_BLOCK, &mask, NULL);
// #endif

	while(!preempt_start);

#ifdef UINTR	    
    uipi_index = uintr_register_sender(uintr_fd, 0);
#endif 

    long long last = _rdtsc(), current;
	long long timeslice = 2L * (timeslice_ns + preempt_overhead);
    while (1) {
        current = _rdtsc();
        if (current - last >= timeslice) {
			last = current;
			preempt_sent ++;
#ifdef SIGNAL
			pthread_kill(tid, SIGUSR1);
			// kill(0, SIGUSR1);
#elif defined(UINTR)
			// printf("senduipi---\n");
        	_senduipi(uipi_index);
#elif defined(CONCORD)
			*cpu_preempt_point = 1;
#endif
        }
    } 
    
	return NULL;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("usage: [timeslice] [value]\n");
		return -EINVAL;
	}
	set_thread_affinity(0);
	timeslice_ns = 1000L * atoi(argv[1]);
	int Value = atoi(argv[2]);
	// printf("timeslice_us: %lld, timeslice: %lld\n", timeslice_us, timeslice);

#ifndef SAFEPOINT 
	pthread_t pt;
	if (pthread_create(&pt, NULL, &timer, NULL)) 
		exit(-1);
	uint64_t deadline = _rdtsc() + 2000 * 1000 * 10;
	while(deadline > _rdtsc());
#endif

	m5op_addr = 0xFFFF0000;
	map_m5_mem();

	uint64_t x = 0;
	m5_reset_stats_addr(50, &&switching_point);
	m5_switch_cpu_addr();
	while (x != 1144000000)
	{
			x++;
			asm("nop");
			asm("nop");
			asm("nop");
			asm("nop");
			asm("nop");
			asm("nop");
	}
switching_point:

#ifdef SIGNAL
	tid = pthread_self();
    struct sigaction action;
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGUSR1, &action, NULL) != 0) {
        printf("signal handler registeration failed");
    }
#elif defined(UINTR)
	// preempt_overhead = 300;
	if (uintr_register_handler(ui_handler, 0))
		exit(-1);

	uintr_fd = uintr_create_fd(TOKEN, 0);
	if (uintr_fd < 0)
		exit(-1);

	// Enable interrupts
	_stui();
	// printf("enable stui\n");
#elif defined(SAFEPOINT)
	// preempt_overhead = 75;
	if (uintr_register_handler(ui_handler, 0))
		exit(-1);
	
	m5_utimer(1000LL*(timeslice_ns+preempt_overhead));
	_stui();

	{
		uint64_t deadline = _rdtsc() + 20000;
		while(deadline > _rdtsc());
	}
	asm volatile ("serialize");

#elif defined(CONCORD)
	concord_preempt_now = 0;
	// concord_preempt_now = 1;
	cpu_preempt_point = &concord_preempt_now;
#endif

	preempt_start = 1;

	start = _rdtsc();

	long long result = bench(Value);

    end = _rdtsc();

	m5_switch_cpu_addr();

	printf("Result: %lld\n", result);
	printf("Time: %lld\n", end - start);
    printf("Preemption: %lld\n", preempt_cnt);
	printf("Sent: %lld\n", preempt_sent);
	// double exe =  1.*(end - start) / 2e9;
	// printf("Execution: %.9f\n", exe);

	return 0;
}

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <sys/prctl.h>

#include "tests/test.h"

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
#define MAXTHREADS 32
#define TIMER_CORE 0
#define TASK_CORE 2
volatile int uintr_cnt = 0, timer_cnt = 0;
int num_threads;
long long quantum;
long long start, end;
int tids[MAXTHREADS], uipi_index[MAXTHREADS];
volatile int uintr_fd[MAXTHREADS];
pthread_t pthreads[MAXTHREADS];
int (*test_ptr)(void);

long long now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void set_thread_affinity(int core) {
	// printf("set_thread_affinity: %d\n", core);
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);
	sched_setaffinity(0, sizeof(mask), &mask);
}

void __attribute__ ((interrupt))
     __attribute__((target("general-regs-only", "inline-all-stringops")))
     ui_handler(struct __uintr_frame *ui_frame,
		unsigned long long vector) {
    ++uintr_cnt;
}

#if defined(SLEEP) || defined(ITIMER) || defined(NOTIMER)
volatile int add = 0;
inline unsigned fib(int n) {
	unsigned f1 = 1, f2 = 1;
	while (n--) {
		int tmp = f1 + f2;
		f1 = f2;
		f2 = tmp;
	}
	return f2;
}
void *compute(void* __attribute__((unused)) arg) {
#if defined(SLEEP) || defined(NOTIMER)
	set_thread_affinity(TIMER_CORE);
#endif
    while (1) {
		add += (fib(10000) > 0);
	}

	return NULL;
}
#endif

void uintr_init_timer() {
	set_thread_affinity(TIMER_CORE);
	    
    int i;
	for (i = 0; i < num_threads; ++i) {
		while( uintr_fd[i] == -1 );
		uipi_index[i] = uintr_register_sender(uintr_fd[i], 0);
		if(uipi_index[i] == -1){
		printf("fd is : %d\n",uintr_fd[i]);
		printf("i: %d\n",i);
		perror("index bad");
		}
	}
	

	// The timer start to run
	start = now();
}

#ifdef SLEEP
void *timer(void* __attribute__((unused)) arg) {
    uintr_init_timer();

    // struct timespec sleepT;
    // sleepT.tv_sec = quantum / (int)1e9;
    // sleepT.tv_nsec = quantum % (int)1e9;

    // long long last = now(), current;
    // while (1) {
	// 	nanosleep(&sleepT, NULL);
	// 	timer_cnt += 1;
	// 	int i;
	// 	for (i = 0; i < num_threads; ++i) {
	// 		_senduipi(uipi_index[i]);
	// 	}
    // } 

	struct timespec ts;
	ts.tv_sec = quantum / (int)1e9;
	ts.tv_nsec = quantum % (int)1e9;

	long long last = now(), current;
    while (1) {
		clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL); 
		timer_cnt += 1;
		int i;
		for (i = 0; i < num_threads; ++i) {
			_senduipi(uipi_index[i]);
		}
    } 
	return NULL;
}

#elif defined(ITIMER)
void signal_handler(int __attribute__((unused)) signo) {
	timer_cnt += 1;
	int i;
	for (i = 0; i < num_threads; ++i) {
		_senduipi(uipi_index[i]);
	}
}

void *timer(void* __attribute__((unused)) arg) {
	uintr_init_timer();
	
	struct sigaction psa;
	psa.sa_handler = signal_handler;
    sigemptyset(&psa.sa_mask);
    psa.sa_flags = 0;
    sigaction(SIGALRM, &psa, NULL);

	// printf("quantum: %lld\n", quantum);
    struct itimerval tv;
    tv.it_interval.tv_sec = quantum / (int)1e9;
    tv.it_interval.tv_usec = (quantum % (int)1e9) / 1000;
    tv.it_value.tv_sec = quantum / (int)1e9;
    tv.it_value.tv_usec = (quantum % (int)1e9) / 1000;
	// printf("us: %ld, %ld\n", tv.it_interval.tv_usec, tv.it_value.tv_usec);
    setitimer(ITIMER_REAL, &tv, NULL);

	compute(NULL);

	return NULL;
}

void block_sig_in_main() {
	// printf("------block_sig_in_main\n");
	sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
}

#elif defined(NOTIMER)
void *timer(void* __attribute__((unused)) arg) {
	start = now();

	compute(NULL);

	return NULL;
}

#else
void *timer(void* __attribute__((unused)) arg) {
	uintr_init_timer();

    long long last = now(), current;
    while (1) {
        current = now();
        if (current - last >= quantum) {
			last = current;
            int i;
			for (i = 0; i < num_threads; ++i) {
				_senduipi(uipi_index[i]);
			}
        }
    } 
    
	return NULL;
}
#endif 


void uintr_init_thread(int tid)
{
	if (uintr_register_handler(ui_handler, 0))
		exit(-1);

	uintr_fd[tid] = uintr_create_fd(TOKEN, 0);
	if (uintr_fd[tid] < 0)
		exit(-1);

	// Enable interrupts
	_stui();

}

void *pthread_entry(void *arg) {
#if defined(ITIMER)
	block_sig_in_main();
#endif 

	int tid = *(int*)arg;

	if(TASK_CORE + tid < 20){
		set_thread_affinity(TASK_CORE + tid);
	}
	else {
	
		set_thread_affinity(TASK_CORE + tid+1);
	}
	uintr_init_thread(tid);

	long long start2 = now();
	test_ptr();
	long long end2 = now();
	// printf("each task: %f sec\n", 1.*(end2-start2)/1e9);
}

void init_task_cores() {
	int i;
	for (i = 0; i < num_threads; ++i) {
		tids[i] = i;
	}
	for (i = 0; i < num_threads; ++i) {
		if (pthread_create(&pthreads[i], NULL, pthread_entry, &tids[i]) != 0) {
			perror("pthread_create failed");
			exit(-1);
		}
	}
}

void init_timer_core() {
#if defined(SLEEP)
    unsigned long slack_value = 100;
	if (prctl(PR_SET_TIMERSLACK, slack_value, 0, 0, 0) == -1) {
        perror("prctl PR_SET_TIMERSLACK");
        exit(-1);
    }
#endif

	// Create the timer thread
	pthread_t pt;
	if (pthread_create(&pt, NULL, &timer, NULL)) 
		exit(-1);

#if defined(SLEEP)
	// Create the compute thread.
	pthread_t pt2;
	if (pthread_create(&pt2, NULL, &compute, NULL)) 
		exit(-1);
#endif 
}

void wait_pthreads() {
	int i;
	for (i = 0; i < num_threads; i++) {
        if (pthread_join(pthreads[i], NULL) != 0) {
            perror("pthread_join failed");
            exit(-1);
        }
    }

	end = now();
}

void print_summary() {
	double exe =  1.*(end - start) / 1e9;
	printf("====== For task core: ======\n");
    printf("Execution: %.9f\n", exe);
    printf("Number of uintrs: %d\n", uintr_cnt);
	
#if defined(SLEEP) || defined(ITIMER) || defined(NOTIMER)
	printf("====== For timer core: ======\n");
	printf("Timer expiration: %d\n", timer_cnt);
	printf("# of tasks: %d\n", add);
	printf("Throughput: %d\n", (int) (add / exe));
#endif
}


int main(int argc, char *argv[]) {
	if (argc != 3) {
        printf("usage: ./timer <number of threads> <quantum>\n");
        return 0;
    }

	num_threads = atoi(argv[1]);
	int quantum_us = atoi(argv[2]);
	quantum = 1LL * quantum_us * 1000; 

	// test_ptr = linpack;
	
	base64_init();
	test_ptr = base64;

	int i;	
	for (i = 0; i < num_threads; ++i) {
		uintr_fd[i] = -1;
	}
	init_task_cores();

	init_timer_core();

#if defined(ITIMER)
	block_sig_in_main();
#endif 
	wait_pthreads();

	print_summary();

	return 0;
}

CFLAGS=-O3 -Wall -Wextra -fPIC -muintr -shared -Im5/src

all:
	$(CC) $(CFLAGS) -Iprograms timer.c -o timer_base.o 
	$(CC) $(CFLAGS) -DSIGNAL -Iprograms timer.c -o timer_signal.o 
	$(CC) $(CFLAGS) -DUINTR -Iprograms timer.c -o timer_uintr.o
	$(CC) $(CFLAGS) -DCONCORD -Iprograms timer.c -o timer_concord.o 
	$(CC) $(CFLAGS) -DSAFEPOINT -Iprograms timer.c -o timer_safepoint.o
	$(CC) $(CFLAGS) -DUINTR -Iprograms timer.c -Wl,-soname,libtimer.so -o libtimer.so

clean:
	rm *.o
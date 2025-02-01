CFLAGS=-g -O3 -muintr

ifeq ($(timer),no)
CFLAGS+=-DNOTIMER
endif

ifeq ($(timer),sleep)
CFLAGS+=-DSLEEP
endif

ifeq ($(timer),itimer)
CFLAGS+=-DITIMER
endif

SRCS = timer.c tests/linpack.c tests/base64.c
OBJS = $(SRCS:.c=.o)

TARGET = timer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(TARGET) $(OBJS)

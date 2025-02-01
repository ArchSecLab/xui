# Makefile for compiling mon.c into mon

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -muintr -O0 -g


# Source file
SRC = mon.c

# Default target
all: send deliver return

# Rule to build the target executable
send: $(SRC)
	$(CC) send.c -o send $(CFLAGS) -DSENDDIFF
deliver: $(SRC)
	$(CC) $(SRC) -o deliver $(CFLAGS) -DSENDMEASURE

return: $(SRC)
	$(CC) $(SRC) -o return $(CFLAGS)
# Clean up build files
clean:
	rm -f $(TARGET)

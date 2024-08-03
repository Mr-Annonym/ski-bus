CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
LDFLAGS=-pthread -lrt # Additional linker flags for semaphores and shared memory
SRCS=ski-bus.c
OBJS=$(SRCS:.c=.o)

all: ski-bus

ski-bus: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o ski-bus

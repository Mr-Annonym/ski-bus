CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -Werror -pedantic
LDFLAGS=-pthread -lrt # Additional linker flags for semaphores and shared memory
SRCS=proj2.c
OBJS=$(SRCS:.c=.o)

all: proj2

proj2: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	rm -f *.o proj2

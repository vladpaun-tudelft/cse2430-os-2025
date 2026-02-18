CC=gcc
CFLAGS=-Wall -Werror -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-parameter  -O0 -std=gnu99 -I/usr/local/include -g -D_GNU_SOURCE
LDFLAGS=-L/usr/local/lib -pthread -lm -lreadline
SOURCES = $(wildcard *.c)


.PHONY: clean default

default:
	echo "Usage: 'make <exe>' where <exe> is the name of the source file without '.c'."
	echo "Example: 'make 2_2'"


queue.o: queue.c
	$(CC) -c $(CFLAGS) queue.c -o queue.o

# Rule to build each executable
%: %.c queue.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


clean:
	$(RM) *.o *.a $(wildcard $(SOURCES:.c=))
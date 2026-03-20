FUSEFLAGS=`pkg-config fuse --cflags --libs`
CC?=gcc
CFLAGS=-Wall -Wextra -pedantic -O2 -std=gnu11 -I/usr/local/include -g -Wno-unused-parameter $(FUSEFLAGS)
LDFLAGS=-L/usr/local/lib -pthread -lm -lfuse -lxxhash

all: os_assignment

os_assignment: os_assignment.o dfs.o dfs-path.o hashtable.a strdup.o
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	$(RM) *.o *.a os_assignment

.PHONY: all clean

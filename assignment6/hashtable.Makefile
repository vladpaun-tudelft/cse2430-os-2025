FUSEFLAGS=`pkg-config fuse --cflags --libs`
CC?=gcc
CFLAGS=-Wall -Wextra -pedantic -O2 -std=gnu11 -I/usr/local/include -g -Wno-unused-parameter $(FUSEFLAGS)
LDFLAGS=-L/usr/local/lib -pthread -lm -lfuse -lxxhash

all: hashtable.a

hashtable.a: hashtable.o
	ar rcs $@ $^

clean:
	$(RM) *.o *.a

.PHONY: all clean

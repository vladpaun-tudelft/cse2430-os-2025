CC=gcc
CFLAGS=-Wall -O0 -std=gnu99 -I/usr/local/include -g
LDFLAGS=-L/usr/local/lib -pthread -lm -lreadline
BINS=4_1 4_2 4_3 


.PHONY: clean default

default:
	echo "Usage: 'make <exe>' where <exe> is the name of the source file without '.c'."
	echo "Example: 'make 4_1'"

4_3: 4_3.c 
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) -DASSIGNMENT_4_3
%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	$(RM) *.o *.a $(BINS)

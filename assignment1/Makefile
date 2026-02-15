CC=gcc
CFLAGS=-Wall -O0 -std=gnu99 -I/usr/local/include -g
LDFLAGS=-L/usr/local/lib -pthread -lm -lreadline
BINS=1_1 1_2 1_3 1_4


.PHONY: clean default

default:
	echo "Usage: 'make <exe>' where <exe> is the name of the source file without '.c'."
	echo "Example: 'make 1_1'"

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	$(RM) *.o *.a $(BINS)

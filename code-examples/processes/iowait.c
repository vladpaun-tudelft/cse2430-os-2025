#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
	printf("Ready -> Running\n");
	// Busy wait to stay running
	double n = 2000000000;
	for (int i = 0; i < 2000000000; i++) {
		n = sqrt(n);
	}

	// Sleep to go to blocked
	printf("Running -> Blocked (sleep)\n");
	sleep(20);

	// Busy wait to stay running
	printf("Blocked -> Running (sleep over)\n");
	for (int i = 0; i < 2000000000; i++) {
		n = sqrt(n);
	}
	return 0;
}

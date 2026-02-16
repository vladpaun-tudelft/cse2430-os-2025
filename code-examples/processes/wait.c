#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
	int pid = 0;
	pid = fork();
	if (pid == 0) {
		printf("[Child] I'm sleeping for 5 seconds...\n");
		sleep(5);
		printf("[Child] I finished sleeping, exiting...\n");
		exit(0);
	} else {
		printf("[Parent] Waiting for the child to terminate...\n");
		wait(NULL);
		printf("[Parent] Done waiting.\n");
		exit(0);
	}
	return 1;
}

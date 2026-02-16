#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int pid = 0;
	pid = fork();
	if (pid == 0) {
		printf("I am the child process\n");
	} else {
		printf("I am the parent process (pid %d)\n", pid);
		exit(1);
	}
	return 1;
}

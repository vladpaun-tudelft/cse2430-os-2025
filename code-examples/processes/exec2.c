#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	int pid;
	pid = fork();
	if (pid == 0) {
		// Child will be consumed by print_something
		char *args[]={"./print_something",NULL};
		execvp(args[0],args);
		printf("Exiting child\n");
	} else {
		printf("Still alive!\n");
		printf("Exiting parent\n");
	}

	return 0;
}

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	char *args[]={"./print_something",NULL};
	execvp(args[0],args);

	printf("Exiting program");
	return 0;
}

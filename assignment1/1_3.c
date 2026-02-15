#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int main() {
    pid_t pid = fork();
 
    if (pid == 0) {
        // child code
        printf("This is the child process.\n");
        system("ls");
    } else {
        //parent code
        printf("This is the parent process.\n");
        wait(NULL);
    }

    return 0;
}
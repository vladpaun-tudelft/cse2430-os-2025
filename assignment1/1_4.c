#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#define MAX_LINE 80 /* The maximum length command */

static int parse(char line[], char *args[]) {
    line[strcspn(line, "\n")] = '\0';

    char *token = strtok(line, " ");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        args[i-1] = NULL;
        return 1;
    }
    return 0;
}

int main(void) {
    char *args[MAX_LINE / 2 + 1]; /* command line arguments */
    int should_run = 1;           /* flag to determine when to exit program */
    while (should_run) {
        printf("osh>");
        fflush(stdout);
        /**
         * After reading user input, the steps are:
         * (1) fork a child process using fork()
         * (2) the child process will invoke execvp()
        * (3) if command did not include &, parent will invoke wait()
        */

        char line[MAX_LINE];
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        
        int ampersand = parse(line, args);
        if (args[0] == NULL) continue;

        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            return 1;
        } else {
            if (!ampersand) waitpid(pid, NULL, 0);
        }
    }
  return 0;
}
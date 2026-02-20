#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main() {
    char *name = "Vlad Paun";
    int student_number = 6152937;

    printf("%s %i\n", name, student_number);

    pid_t pid = getpid();
    printf("%ld\n", (long int)pid);

    return 0;
}
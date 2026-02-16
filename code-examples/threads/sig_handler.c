#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sighup_handler() { printf("the program has received a SIGHUP signal\n"); }

int main(void) {
  int i = 0;

  printf("my pid is %d\n", getpid());

  signal(SIGHUP, sighup_handler);
  while (1) {
    i++;
    printf("%d\n", i);
    sleep(3);
  }
}
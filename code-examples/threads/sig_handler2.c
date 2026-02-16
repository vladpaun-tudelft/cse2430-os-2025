#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void sigterm_handler() { printf("ignoring the TERM signal :-)\n"); }

void sigint_handler() { printf("\nignoring the INT signal ^^\n"); }

int main(void) {
  int i = 0;

  printf("my pid is %d\n", getpid());

  signal(SIGTERM, sigterm_handler);
  signal(SIGINT, sigint_handler);

  while (1) {
    i++;
    printf("%d\n", i);
    sleep(3);
  }

  return 0;
}
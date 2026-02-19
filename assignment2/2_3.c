#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <unistd.h>

#include "util.h"



void run_process(long input, int nice_value) {
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(0, &set);
  sched_setaffinity(0, sizeof(set), &set);

  setpriority(PRIO_PROCESS, 0, nice_value);
  int result = function_1(input);

  printf("Niceness: %d, Result: %d\n", nice_value, result);
  fflush(stdout);

  return;
}

int main() {
  char line[256];

  printf("Enter a single positive number: ");
  fflush(stdout);

  if (fgets(line, sizeof(line), stdin) == NULL)
    return 1;

  long val = atol(line);
  if (val <= 0)
    return 1;

  pid_t pid1 = fork();
  if (pid1 < 0)
    return 1;

  if (pid1 == 0) {
    //first child
    run_process(val, 0);
    return 0;
  }

  pid_t pid2 = fork();
  if (pid2 < 0)
    return 1;

  if (pid2 == 0) {
    //second child
    run_process(val, 10);
    return 0;
  }

  //only parent
  run_process(val, 19);

  return 0;
}
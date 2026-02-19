#include "queue.h"
#include "util.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *thread_1_and_2_func(void *input);
void *thread3_func(void *input);

int main(void) {
  int ret = 0;
  pthread_t t1, t2, t3;

  pthread_barrier_t barrier;
  if (pthread_barrier_init(&barrier, NULL, 3) != 0)
    return 1;

  Queue *q1_in = malloc(sizeof(Queue));
  Queue *q2_in = malloc(sizeof(Queue));
  Queue *q1_out = malloc(sizeof(Queue));
  Queue *q2_out = malloc(sizeof(Queue));
  if (!q1_in || !q2_in || !q1_out || !q2_out)
    return 1;

  initQueue(q1_in);
  initQueue(q2_in);
  initQueue(q1_out);
  initQueue(q2_out);

  arguments *args1 = malloc(sizeof(arguments));
  arguments *args2 = malloc(sizeof(arguments));
  arguments *args3 = malloc(sizeof(arguments));
  if (!args1 || !args2 || !args3)
    return 1;

  args1->queue_1 = q1_in;
  args1->queue_2 = q1_out;
  args1->barrier = &barrier;
  args1->func = function_1;

  args2->queue_1 = q2_in;
  args2->queue_2 = q2_out;
  args2->barrier = &barrier;
  args2->func = function_2;

  args3->queue_1 = q1_out;
  args3->queue_2 = q2_out;
  args3->barrier = &barrier;
  args3->func = function_3;

  if (pthread_create(&t1, NULL, thread_1_and_2_func, args1) != 0)
    return 1;
  if (pthread_create(&t2, NULL, thread_1_and_2_func, args2) != 0)
    return 1;
  if (pthread_create(&t3, NULL, thread3_func, args3) != 0)
    return 1;

  char line[256];
  while (fgets(line, sizeof line, stdin) != NULL) {
    int val = atoi(line);

    if (val < 1 || val > 100) {
      if (val == 0)
        ret = 1;
      addToQueue(q1_in, -2);
      addToQueue(q2_in, -2);
      break;
    }

    addToQueue(q1_in, val);
    addToQueue(q2_in, val);
  }

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);

  pthread_barrier_destroy(&barrier);

  free(args1);
  free(args2);
  free(args3);
  free(q1_in);
  free(q2_in);
  free(q1_out);
  free(q2_out);

  return ret;
}

void *thread_1_and_2_func(void *input) {
  arguments *args = (arguments *)input;
  Queue *input_queue = args->queue_1;
  Queue *output_queue = args->queue_2;

  int val;
  int result;
  while (1) {
    removeFromQueue(input_queue, &val);
    if (val == -1)
      continue;
    if (val < 0) {
      addToQueue(output_queue, val);
      pthread_barrier_wait(args->barrier);
      break;
    }

    result = (*args->func)(val);
    addToQueue(output_queue, result);
    pthread_barrier_wait(args->barrier);
  }

  return NULL;
}
void *thread3_func(void *input) {
  arguments *args = (arguments *)input;
  Queue *input_queue_1 = args->queue_1;
  Queue *input_queue_2 = args->queue_2;

  int val1;
  int val2;
  while (1) {
    pthread_barrier_wait(args->barrier);

    removeFromQueue(input_queue_1, &val1);
    removeFromQueue(input_queue_2, &val2);

    if (val1 < 0 || val2 < 0)
      break;

    int result = (*args->func)(val1 * val2);
    printf("%d\n", result);
    fflush(stdout);
  }

  return NULL;
}
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *long_calculation(void* input) {
  int x = *(int *) input;
  free(input);

  sleep(1);

  printf("%d", x+1);
  fflush(stdout);
  return NULL;
}

int main() {
    int nThreads = 0;
    pthread_t threads[10];
    char line[256];

    while (fgets(line, sizeof(line), stdin) != NULL) {
      int x = atoi(line);

      if (nThreads == 10 || x < 0 || x > 99)
        break;

      int *input = malloc(sizeof(int));
      *input = x;

      if (pthread_create(&threads[nThreads], NULL, &long_calculation,
                         (void *)input) != 0) {
        free(input);
        break;
      } else
        nThreads++;
    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(threads[i],NULL);
    }

}

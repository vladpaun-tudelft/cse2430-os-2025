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

    pthread_t current_threads_running[10];

    while (1) {
        char line[256];
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        int x = atoi(line);
        

        if (x < 0 || x > 99) {
            break;
        }

        if (nThreads == 10) {
            pthread_join(current_threads_running[0],NULL);
            for (int i = 1; i < nThreads; i++) {
                current_threads_running[i - 1] = current_threads_running[i];
            }
            nThreads--;
        }

        int *input = malloc(sizeof(int));
        *input = x;

        if (pthread_create(&current_threads_running[nThreads],NULL, &long_calculation, (void*)input) != 0) {
            free(input);
            break;
        }
        nThreads++;

    }

    for (int i = 0; i < nThreads; i++) {
        pthread_join(current_threads_running[i],NULL);
    }

}

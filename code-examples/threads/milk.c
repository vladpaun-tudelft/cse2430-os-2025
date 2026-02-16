#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

static const char *P[2] = {"you", "your roommate"};

int milk = 0;

void *person(void *arg) {
  uintptr_t i = (uintptr_t)arg;
  printf("%s looks into the fridge\n", P[i]);
  if (milk == 0) {
    printf("%s: no milk, I go and buy some\n", P[i]);
    milk++;
  } else {
    printf("%s: there is milk, no need to buy more milk\n", P[i]);
  }

  return 0;
}

int main() {
  uintptr_t i = 0;
  int j = 0;
  pthread_t t[10];

  for (i = 0; i < 2; i++) {
    pthread_create(&t[i], NULL, person, (void *)i);
  }

  for (j = 0; j < 2; j++) {
    pthread_join(t[j], NULL);
  }

  printf("we ended up with %d bottles of milk\n", milk);

  return 0;
}
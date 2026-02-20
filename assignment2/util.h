#pragma once

#include "queue.h"

#define ITERATIONS 500000000

int function_1(int arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        if (arg % 2 == 0) {
            arg = arg / 2;
        } else {
            arg = 3 * arg + 1;
        }
    }

    return arg;
}

int function_2(int arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        arg = (arg * 42) % 71;
    }

    return arg;
}

int function_3(int arg) { return arg + 10; }

typedef struct {
    Queue *queue_1;
    Queue *queue_2;
    pthread_barrier_t *barrier;
    int (*func)(int arg);
} arguments;

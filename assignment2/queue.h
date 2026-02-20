/**
 * @file queue.h
 *
 * A queue datastructure.
 *
 * This header defines the interface of the queue. You should not have to
 * modify this file; see queue.c instead.
 */

#pragma once

#include <pthread.h>

/**
 * A node in the queue.
 */
typedef struct Node {
    int val;
    struct Node *next;
} Node;

/**
 * A queue.
 */
typedef struct Queue {
    Node *front; ///< The first node of the queue or NULL if the queue is empty.
    Node *back;  ///< The last node of the queue or NULL if the queue is empty.
    pthread_mutex_t mutex; ///< The mutex guarding the queue when multiple
                           ///< threads are active.
} Queue;

void addToQueue(Queue *queue, int val);

void removeFromQueue(Queue *queue, int *val);

int queueSize(Queue *queue);

void initQueue(Queue *queue);

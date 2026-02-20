#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int thread_count = 0; // increment this
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int *array;
    int first;
    int last;
} arguments;

void init(int *array, int length) {
    srand(4);
    for (int i = 0; i < length; i++)
        array[i] = rand() % 1234 + 1;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int pivot(int *array, int first, int last) {
    int p = first;
    int pivotElement = array[first];
    for (int i = first + 1; i <= last; i++) {
        if (array[i] <= pivotElement) {
            p++;
            swap(&(array[i]), &(array[p]));
        }
    }
    swap(&(array[p]), &(array[first]));
    return p;
}

void serial_quicksort(arguments *args) {
    if (args->first < args->last) {
        arguments args1, args2;
        int pivotElement;
        pivotElement = pivot(args->array, args->first, args->last);

        args1.array = args->array;
        args1.first = args->first;
        args1.last = pivotElement - 1;
        args2.array = args->array;
        args2.first = pivotElement + 1;
        args2.last = args->last;

        serial_quicksort(&args1);
        serial_quicksort(&args2);
    }
}

int checkFn(int *array, int length) {
    for (int i = 0; i < length - 1; i++) {
        if (array[i] > array[i + 1]) {
            printf("array[%d] > array[%d]\n", i, i + 1);
            return 0;
        }
    }
    return 1;
}

void *quicksort(void *v_args) {
    arguments *args = (arguments *)v_args;

    if (args->first < args->last) {
        arguments args1, args2;
        int pivotElement;
        pivotElement = pivot(args->array, args->first, args->last);

        args1.array = args->array;
        args1.first = args->first;
        args1.last = pivotElement - 1;
        args2.array = args->array;
        args2.first = pivotElement + 1;
        args2.last = args->last;

        if (args->last - args->first < 10000) {
            serial_quicksort(&args1);
            serial_quicksort(&args2);
        } else {
            pthread_t threads[2];

            pthread_mutex_lock(&mutex);
            thread_count++;
            thread_count++;
            pthread_mutex_unlock(&mutex);

            pthread_create(&threads[0], NULL, quicksort, &args1);
            pthread_create(&threads[1], NULL, quicksort, &args2);

            pthread_join(threads[0], NULL);
            pthread_join(threads[1], NULL);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int length;
    int *toBeSorted = NULL;
    arguments args;

    if (argc < 2) {
        length = 6000000;
    } else {
        length = atoi(argv[1]);
    }
    toBeSorted = (int *)calloc(length, sizeof(int));
    if (toBeSorted == NULL) {
        printf("Failed to allocate the array to be sorted!\n");
        exit(-1);
    }

    init(toBeSorted, length);
    printf("Sorting an array of %d elements.\n", length);

    args.array = toBeSorted;
    args.first = 0;
    args.last = length - 1;
    quicksort(&args);

    if (!checkFn(toBeSorted, length)) {
        printf("validation failed!\n");
    } else {
        printf("validation is successfull!\n");
    }

    free(toBeSorted);
}

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

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

void *quicksort_with_pipes(void *v_args) {
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

        if (args->last - args->first < 10) {
            serial_quicksort(&args1);
            serial_quicksort(&args2);
            return NULL;
        }

        int p[2];
        if (pipe(p) < 0)
            exit(1);

        pid_t pid1 = fork();
        if (pid1 < 0)
            exit(1);

        if (pid1 == 0) {
            // first child
            close(p[0]);

            serial_quicksort(&args1);

            int count1 = args1.last - args1.first + 1;
            if (count1 > 0) {
                char *point = (char *)&args1.array[args1.first];
                size_t bytes = (size_t)count1 * sizeof(args1.array[0]);

                while (bytes > 0) {
                    ssize_t w = write(p[1], point, bytes);
                    if (w <= 0) {
                        close(p[1]);
                        exit(0);
                    }
                    point += (size_t)w;
                    bytes -= (size_t)w;
                }
            }
            close(p[1]);
            exit(0);
        }
        // daddy
        close(p[1]);
        serial_quicksort(&args2);

        int count1 = args1.last - args1.first + 1;
        if (count1 > 0) {
            char *point = (char *)&args1.array[args1.first];
            size_t bytes = (size_t)count1 * sizeof(args1.array[0]);

            while (bytes > 0) {
                ssize_t r = read(p[0], point, bytes);
                if (r <= 0)
                    break;
                point += (size_t)r;
                bytes -= (size_t)r;
            }
        }
        close(p[0]);
        waitpid(pid1, NULL, 0);
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
    quicksort_with_pipes(&args);

    if (!checkFn(toBeSorted, length)) {
        printf("validation failed!\n");
    } else {
        printf("validation is successfull!\n");
    }

    free(toBeSorted);
}

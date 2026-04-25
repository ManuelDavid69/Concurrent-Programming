#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    pthread_barrier_t barrier;
} Rendezvous;

typedef struct {
    Rendezvous *shared;
    int index;
} ThreadArgument;

static void *worker(void *argument)
{
    ThreadArgument *thread = argument;

    pc_seed();
    printf("Thread %d: avant la barrière\n", thread->index);
    fflush(stdout);
    pc_sleep(rand_range(1, 3));
    pthread_barrier_wait(&thread->shared->barrier);
    printf("Thread %d: barrière franchie\n", thread->index);
    fflush(stdout);
    return NULL;
}

int main(int argc, char **argv)
{
    int parties = 4;
    pthread_t *threads;
    ThreadArgument *arguments;
    Rendezvous rendezvous;
    int index;

    if (argc >= 2 && (!parse_int(argv[1], &parties) || parties < 1 || parties > 1000)) {
        fprintf(stderr, "usage: %s [threads]\n", argv[0]);
        return EXIT_FAILURE;
    }
    threads = xmalloc((size_t)parties * sizeof(*threads));
    arguments = xmalloc((size_t)parties * sizeof(*arguments));
    if (pthread_barrier_init(&rendezvous.barrier, NULL, (unsigned int)parties) != 0) {
        die("pthread_barrier_init failed");
    }
    for (index = 0; index < parties; index++) {
        arguments[index].shared = &rendezvous;
        arguments[index].index = index;
        if (pthread_create(&threads[index], NULL, worker, &arguments[index]) != 0) {
            die("pthread_create failed");
        }
    }
    for (index = 0; index < parties; index++) {
        pthread_join(threads[index], NULL);
    }
    pthread_barrier_destroy(&rendezvous.barrier);
    free(arguments);
    free(threads);
    return EXIT_SUCCESS;
}

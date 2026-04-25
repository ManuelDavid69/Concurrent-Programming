#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int parties;
    int arrived;
    int generation;
} Rendezvous;

typedef struct {
    Rendezvous *shared;
    int index;
} ThreadArgument;

static void *worker(void *argument)
{
    ThreadArgument *thread = argument;
    Rendezvous *rendezvous = thread->shared;

    pc_seed();
    printf("Thread %d: avant le rendez-vous\n", thread->index);
    fflush(stdout);
    pc_sleep(rand_range(1, 3));

    pthread_mutex_lock(&rendezvous->mutex);
    {
        int generation = rendezvous->generation;
        rendezvous->arrived++;
        if (rendezvous->arrived == rendezvous->parties) {
            rendezvous->arrived = 0;
            rendezvous->generation++;
            pthread_cond_broadcast(&rendezvous->condition);
        } else {
            while (generation == rendezvous->generation) {
                pthread_cond_wait(&rendezvous->condition, &rendezvous->mutex);
            }
        }
    }
    pthread_mutex_unlock(&rendezvous->mutex);

    printf("Thread %d: rendez-vous atteint\n", thread->index);
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
    rendezvous.parties = parties;
    rendezvous.arrived = 0;
    rendezvous.generation = 0;
    pthread_mutex_init(&rendezvous.mutex, NULL);
    pthread_cond_init(&rendezvous.condition, NULL);

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
    pthread_cond_destroy(&rendezvous.condition);
    pthread_mutex_destroy(&rendezvous.mutex);
    free(arguments);
    free(threads);
    return EXIT_SUCCESS;
}

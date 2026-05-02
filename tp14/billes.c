#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    int available;
    int maximum;
    int workers_done;
    int workers_expected;
    int stop_controller;
    int controller_checks;
    int invariant_error;
} BallManager;

typedef struct {
    BallManager *manager;
    int identifier;
    int demand;
    int iterations;
} WorkerArgument;

static void demand(BallManager *manager, int count)
{
    pthread_mutex_lock(&manager->mutex);
    while (manager->available < count) {
        pthread_cond_wait(&manager->condition, &manager->mutex);
    }
    manager->available -= count;
    pthread_mutex_unlock(&manager->mutex);
}

static void give_back(BallManager *manager, int count)
{
    pthread_mutex_lock(&manager->mutex);
    manager->available += count;
    pthread_cond_broadcast(&manager->condition);
    pthread_mutex_unlock(&manager->mutex);
}

static void *worker(void *argument)
{
    WorkerArgument *worker_argument = argument;
    BallManager *manager = worker_argument->manager;

    pc_seed();
    for (int iteration = 0; iteration < worker_argument->iterations; iteration++) {
        int remaining;

        demand(manager, worker_argument->demand);
        pthread_mutex_lock(&manager->mutex);
        remaining = manager->available;
        pthread_mutex_unlock(&manager->mutex);
        printf("Travailleur %d: %d bille(s) obtenue(s) (disponible %d)\n",
               worker_argument->identifier, worker_argument->demand, remaining);
        fflush(stdout);
        pc_sleep(0.2 + 0.05 * worker_argument->identifier);
        give_back(manager, worker_argument->demand);
    }
    pthread_mutex_lock(&manager->mutex);
    manager->workers_done++;
    pthread_cond_broadcast(&manager->condition);
    pthread_mutex_unlock(&manager->mutex);
    printf("Travailleur %d: terminé\n", worker_argument->identifier);
    return NULL;
}

static void *controller(void *argument)
{
    BallManager *manager = argument;

    for (;;) {
        bool stop;
        int available;

        pthread_mutex_lock(&manager->mutex);
        available = manager->available;
        stop = manager->stop_controller != 0;
        manager->controller_checks++;
        if (available < 0 || available > manager->maximum) {
            manager->invariant_error = 1;
            printf("Contrôleur: invariant violé (%d)\n", available);
            fflush(stdout);
        }
        pthread_mutex_unlock(&manager->mutex);
        if (stop) {
            break;
        }
        pc_sleep(1.0);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int worker_count = 4;
    int maximum = 9;
    int iterations = 3;
    int demands[64] = {4, 3, 5, 2};
    BallManager manager = {0};
    WorkerArgument *arguments;
    pthread_t *workers;
    pthread_t controller_thread;
    int demand_count = 4;

    if (argc >= 2 && (!parse_int(argv[1], &worker_count) || worker_count < 1
                      || worker_count > 64)) {
        fprintf(stderr, "usage: %s [workers] [max_balls] [iterations] [demands]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 3 && (!parse_int(argv[2], &maximum) || maximum < 1)) {
        fprintf(stderr, "invalid maximum\n");
        return EXIT_FAILURE;
    }
    if (argc >= 4 && (!parse_int(argv[3], &iterations) || iterations < 0)) {
        fprintf(stderr, "invalid number of iterations\n");
        return EXIT_FAILURE;
    }
    if (argc >= 5) {
        char *copy = xstrdup(argv[4]);
        char *token;
        char *save = NULL;
        demand_count = 0;
        for (token = strtok_r(copy, ",", &save); token != NULL && demand_count < 64;
             token = strtok_r(NULL, ",", &save)) {
            int value;
            if (!parse_int(token, &value) || value < 0) {
                fprintf(stderr, "invalid demand: %s\n", token);
                free(copy);
                return EXIT_FAILURE;
            }
            demands[demand_count++] = value;
        }
        free(copy);
        if (demand_count == 0) {
            fprintf(stderr, "at least one demand is required\n");
            return EXIT_FAILURE;
        }
    }
    if (worker_count > 64) {
        worker_count = 64;
    }
    for (int index = 0; index < worker_count; index++) {
        if (demands[index % demand_count] > maximum) {
            fprintf(stderr, "demand %d exceeds the maximum of %d balls\n",
                    demands[index % demand_count], maximum);
            return EXIT_FAILURE;
        }
    }

    manager.available = maximum;
    manager.maximum = maximum;
    manager.workers_expected = worker_count;
    pthread_mutex_init(&manager.mutex, NULL);
    pthread_cond_init(&manager.condition, NULL);
    workers = xmalloc((size_t)worker_count * sizeof(*workers));
    arguments = xmalloc((size_t)worker_count * sizeof(*arguments));

    pthread_create(&controller_thread, NULL, controller, &manager);
    for (int index = 0; index < worker_count; index++) {
        arguments[index].manager = &manager;
        arguments[index].identifier = index;
        arguments[index].demand = demands[index % demand_count];
        arguments[index].iterations = iterations;
        pthread_create(&workers[index], NULL, worker, &arguments[index]);
    }
    for (int index = 0; index < worker_count; index++) {
        pthread_join(workers[index], NULL);
    }
    pthread_mutex_lock(&manager.mutex);
    manager.stop_controller = 1;
    pthread_mutex_unlock(&manager.mutex);
    pthread_join(controller_thread, NULL);

    printf("Contrôleur: %d vérification(s), %s\n", manager.controller_checks,
           manager.invariant_error ? "invariant violé" : "invariant respecté");
    pthread_cond_destroy(&manager.condition);
    pthread_mutex_destroy(&manager.mutex);
    free(arguments);
    free(workers);
    return manager.invariant_error ? EXIT_FAILURE : EXIT_SUCCESS;
}

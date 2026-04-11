#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int parties;
    int semaphores;
    int memory;
    RendezVous *state;
    int current_generation;

    if (argc != 2 || !parse_int(argv[1], &parties) || parties < 1 || parties > 1000) {
        fprintf(stderr, "usage: %s N (number of participants)\n", argv[0]);
        return EXIT_FAILURE;
    }
    semaphores = sem_open(TP10_RDV_SEM_KEY, RDV_SEM_COUNT);
    memory = shm_open(TP10_RDV_SHM_KEY, sizeof(RendezVous));
    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    state = shm_attach(memory);
    if (state == NULL) {
        die("cannot attach the shared state");
    }

    printf("Processus %ld: avant le rendez-vous (N=%d)\n", (long)getpid(), parties);
    fflush(stdout);
    pc_seed();
    pc_sleep(rand_range(1, 3));

    if (sem_p(semaphores, RDV_MUTEX) < 0) {
        die("sem_p failed");
    }
    current_generation = state->generation;
    state->count++;
    if (state->count == parties) {
        state->count = 0;
        state->generation = current_generation + 1;
        if (sem_v(semaphores, RDV_MUTEX) < 0) {
            die("sem_v failed");
        }
        for (int index = 0; index < parties - 1; index++) {
            sem_v(semaphores, RDV_TURNSTILE_0 + current_generation % 2);
        }
    } else {
        if (sem_v(semaphores, RDV_MUTEX) < 0) {
            die("sem_v failed");
        }
        if (sem_p(semaphores, RDV_TURNSTILE_0 + current_generation % 2) < 0) {
            die("sem_p failed");
        }
    }
    printf("Processus %ld: rendez-vous atteint, je continue\n", (long)getpid());
    fflush(stdout);
    shm_detach(state);
    return EXIT_SUCCESS;
}

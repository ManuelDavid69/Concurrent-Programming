#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int semaphores = sem_open(TP08_SEM_KEY, SEM_COUNT);
    int memory = shm_open(TP08_SHM_KEY, sizeof(Etat));
    Etat *state;
    bool immediate;

    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    state = shm_attach(memory);
    if (state == NULL) {
        die("cannot attach the shared state");
    }
    printf("Recepteur %ld: démarre\n", (long)getpid());
    fflush(stdout);

    if (sem_p(semaphores, SEM_MUTEX) < 0) {
        die("sem_p failed");
    }
    if (state->emetteurs_attente >= 1 && state->recepteurs_attente >= 1) {
        state->emetteurs_attente--;
        state->recepteurs_attente--;
        immediate = true;
    } else {
        state->recepteurs_attente++;
        immediate = false;
    }
    if (sem_v(semaphores, SEM_MUTEX) < 0) {
        die("sem_v failed");
    }

    if (immediate) {
        sem_v(semaphores, SEM_EMETTEUR);
        sem_v(semaphores, SEM_RECEPTEUR);
        printf("Recepteur %ld: libéré avec un émetteur et un autre récepteur\n",
               (long)getpid());
    } else {
        printf("Recepteur %ld: en attente\n", (long)getpid());
        fflush(stdout);
        if (sem_p(semaphores, SEM_RECEPTEUR) < 0) {
            die("sem_p failed");
        }
        printf("Recepteur %ld: libéré\n", (long)getpid());
    }
    fflush(stdout);
    shm_detach(state);
    return EXIT_SUCCESS;
}

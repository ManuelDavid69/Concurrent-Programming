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
    int message;

    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    state = shm_attach(memory);
    if (state == NULL) {
        die("cannot attach the shared state");
    }
    pc_seed();
    message = rand_range(1000, 9999);
    printf("Emetteur %ld: démarre avec le message %d\n", (long)getpid(), message);
    fflush(stdout);

    if (sem_p(semaphores, SEM_MUTEX) < 0) {
        die("sem_p failed");
    }
    if (state->recepteurs_attente >= 2) {
        state->recepteurs_attente -= 2;
        immediate = true;
    } else {
        state->emetteurs_attente++;
        immediate = false;
    }
    if (sem_v(semaphores, SEM_MUTEX) < 0) {
        die("sem_v failed");
    }

    if (immediate) {
        sem_v(semaphores, SEM_RECEPTEUR);
        sem_v(semaphores, SEM_RECEPTEUR);
        printf("Emetteur %ld: deux récepteurs présents, message %d envoyé\n",
               (long)getpid(), message);
    } else {
        printf("Emetteur %ld: en attente d'un deuxième récepteur\n", (long)getpid());
        fflush(stdout);
        if (sem_p(semaphores, SEM_EMETTEUR) < 0) {
            die("sem_p failed");
        }
        printf("Emetteur %ld: libéré, message %d envoyé\n", (long)getpid(), message);
    }
    fflush(stdout);
    shm_detach(state);
    return EXIT_SUCCESS;
}

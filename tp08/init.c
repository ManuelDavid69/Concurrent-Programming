#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    unsigned short values[SEM_COUNT] = {0};
    int semaphores;
    int memory;
    Etat *state;

    values[SEM_FIRST] = 0;
    values[SEM_RDV2_BASE] = 0;
    values[SEM_RDV2_BASE + 1] = 0;
    values[SEM_RDV3_BASE] = 0;
    values[SEM_RDV3_BASE + 1] = 0;
    values[SEM_RDV3_BASE + 2] = 0;
    values[SEM_MUTEX] = 1;
    values[SEM_EMETTEUR] = 0;
    values[SEM_RECEPTEUR] = 0;

    sem_remove_key(TP08_SEM_KEY);
    shm_remove_key(TP08_SHM_KEY);

    semaphores = sem_create(TP08_SEM_KEY, SEM_COUNT, values);
    if (semaphores < 0) {
        die("cannot create the semaphore set");
    }
    memory = shm_create(TP08_SHM_KEY, sizeof(Etat));
    if (memory < 0) {
        sem_remove(semaphores);
        die("cannot create the shared state");
    }
    state = shm_attach(memory);
    if (state == NULL) {
        sem_remove(semaphores);
        shm_remove(memory);
        die("cannot attach the shared state");
    }
    memset(state, 0, sizeof(*state));
    shm_detach(state);
    printf("Ressources du TP 8 créées (sémaphores et état partagé).\n");
    return EXIT_SUCCESS;
}

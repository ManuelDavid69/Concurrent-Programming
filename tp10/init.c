#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int capacity = 5;
    int producers = 1;
    unsigned short rdv_values[RDV_SEM_COUNT] = {1, 0, 0};
    unsigned short pc_values[PC_SEM_COUNT];
    unsigned short exch_values[EXCH_SEM_COUNT] = {0, 0, 0, 0};
    int rdv_sem;
    int pc_sem;
    int exch_sem;
    int rdv_shm;
    int pc_shm;
    int exch_shm;
    RendezVous *rendezvous;
    Buffer *buffer;
    Echange *exchange;

    if (argc >= 2 && (!parse_int(argv[1], &capacity) || capacity <= 0 || capacity > 10000)) {
        fprintf(stderr, "usage: %s [capacity] [producers]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 3 && (!parse_int(argv[2], &producers) || producers <= 0 || producers > 1000)) {
        fprintf(stderr, "usage: %s [capacity] [producers]\n", argv[0]);
        return EXIT_FAILURE;
    }

    sem_remove_key(TP10_RDV_SEM_KEY);
    sem_remove_key(TP10_PC_SEM_KEY);
    sem_remove_key(TP10_EXCH_SEM_KEY);
    shm_remove_key(TP10_RDV_SHM_KEY);
    shm_remove_key(TP10_PC_SHM_KEY);
    shm_remove_key(TP10_EXCH_SHM_KEY);

    pc_values[PC_MUTEX] = 1;
    pc_values[PC_EMPTY] = (unsigned short)capacity;
    pc_values[PC_FULL] = 0;

    rdv_sem = sem_create(TP10_RDV_SEM_KEY, RDV_SEM_COUNT, rdv_values);
    pc_sem = sem_create(TP10_PC_SEM_KEY, PC_SEM_COUNT, pc_values);
    exch_sem = sem_create(TP10_EXCH_SEM_KEY, EXCH_SEM_COUNT, exch_values);
    rdv_shm = shm_create(TP10_RDV_SHM_KEY, sizeof(RendezVous));
    pc_shm = shm_create(TP10_PC_SHM_KEY, sizeof(Buffer) + (size_t)capacity * sizeof(int));
    exch_shm = shm_create(TP10_EXCH_SHM_KEY, sizeof(Echange));

    if (rdv_sem < 0 || pc_sem < 0 || exch_sem < 0 || rdv_shm < 0 || pc_shm < 0
        || exch_shm < 0) {
        sem_remove(rdv_sem);
        sem_remove(pc_sem);
        sem_remove(exch_sem);
        shm_remove(rdv_shm);
        shm_remove(pc_shm);
        shm_remove(exch_shm);
        die("cannot create the TP 10 resources");
    }

    rendezvous = shm_attach(rdv_shm);
    buffer = shm_attach(pc_shm);
    exchange = shm_attach(exch_shm);
    if (rendezvous == NULL || buffer == NULL || exchange == NULL) {
        die("cannot attach the shared segments");
    }
    memset(rendezvous, 0, sizeof(*rendezvous));
    memset(buffer, 0, sizeof(Buffer) + (size_t)capacity * sizeof(int));
    buffer->capacity = capacity;
    buffer->producers_expected = producers;
    memset(exchange, 0, sizeof(*exchange));

    shm_detach(rendezvous);
    shm_detach(buffer);
    shm_detach(exchange);
    printf("Ressources du TP 10 créées (capacité %d, %d producteur(s)).\n", capacity, producers);
    return EXIT_SUCCESS;
}

#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int identifier;
    int semaphores;
    int index;

    if (argc != 2 || !parse_int(argv[1], &identifier) || identifier < 0 || identifier > 1) {
        fprintf(stderr, "usage: %s 0|1\n", argv[0]);
        return EXIT_FAILURE;
    }
    semaphores = sem_open(TP08_SEM_KEY, SEM_COUNT);
    if (semaphores < 0) {
        die("semaphore set not found; run init first");
    }
    pc_seed();
    printf("Processus %d: traitement avant le rendez-vous\n", identifier);
    fflush(stdout);
    pc_sleep(rand_range(1, 3));

    for (index = 0; index < 2; index++) {
        if (index != identifier && sem_v(semaphores, SEM_RDV2_BASE + index) < 0) {
            die("sem_v failed");
        }
    }
    printf("Processus %d: arrivé au rendez-vous, j'attends l'autre\n", identifier);
    fflush(stdout);
    if (sem_p(semaphores, SEM_RDV2_BASE + identifier) < 0) {
        die("sem_p failed");
    }
    printf("Processus %d: rendez-vous atteint, je continue\n", identifier);
    return EXIT_SUCCESS;
}

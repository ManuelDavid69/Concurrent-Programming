#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int exchanges = 0;
    int semaphores = sem_open(TP10_EXCH_SEM_KEY, EXCH_SEM_COUNT);
    int memory = shm_open(TP10_EXCH_SHM_KEY, sizeof(Echange));
    Echange *exchange;

    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    exchange = shm_attach(memory);
    if (exchange == NULL) {
        die("cannot attach the shared cells");
    }

    for (;;) {
        if (sem_p(semaphores, EXCH_P1) < 0 || sem_p(semaphores, EXCH_P2) < 0) {
            die("sem_p failed");
        }
        if (exchange->first > exchange->second) {
            int temporary = exchange->first;
            exchange->first = exchange->second;
            exchange->second = temporary;
            exchange->third = 0;
            exchanges++;
        } else {
            exchange->third = -1;
        }
        if (sem_v(semaphores, EXCH_P0_TO_P1) < 0
            || sem_v(semaphores, EXCH_P0_TO_P2) < 0) {
            die("sem_v failed");
        }
        if (exchange->third == -1) {
            break;
        }
    }
    printf("P0: %d échange(s), les tableaux sont ordonnés\n", exchanges);
    shm_detach(exchange);
    return EXIT_SUCCESS;
}

#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int semaphores = sem_open(TP08_SEM_KEY, SEM_COUNT);

    if (semaphores < 0) {
        die("semaphore set not found; run init first");
    }
    pc_seed();
    printf("First: début\n");
    fflush(stdout);
    pc_sleep(rand_range(3, 5));
    printf("First: fin de l'attente\n");
    fflush(stdout);
    if (sem_v(semaphores, SEM_FIRST) < 0) {
        die("sem_v failed");
    }
    return EXIT_SUCCESS;
}

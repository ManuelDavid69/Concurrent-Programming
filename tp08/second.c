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
    printf("Second: début, j'attends first\n");
    fflush(stdout);
    if (sem_p(semaphores, SEM_FIRST) < 0) {
        die("sem_p failed");
    }
    printf("Second: first a terminé son attente\n");
    fflush(stdout);
    pc_sleep(rand_range(1, 4));
    printf("Second: fin\n");
    fflush(stdout);
    return EXIT_SUCCESS;
}

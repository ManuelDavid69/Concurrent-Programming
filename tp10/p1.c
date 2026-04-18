#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TABLE_SIZE 8

static void print_table(const char *label, const int *table)
{
    printf("%s:", label);
    for (int index = 0; index < TABLE_SIZE; index++) {
        printf(" %d", table[index]);
    }
    printf("\n");
}

int main(void)
{
    int table[TABLE_SIZE] = {20, 30, 40, 50, 60, 70, 80, 90};
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
        int maximum_index = 0;

        for (int index = 1; index < TABLE_SIZE; index++) {
            if (table[index] > table[maximum_index]) {
                maximum_index = index;
            }
        }
        exchange->first = table[maximum_index];
        if (sem_v(semaphores, EXCH_P1) < 0) {
            die("sem_v failed");
        }
        if (sem_p(semaphores, EXCH_P0_TO_P1) < 0) {
            die("sem_p failed");
        }
        if (exchange->third == -1) {
            break;
        }
        table[maximum_index] = exchange->first;
    }
    print_table("P1", table);
    shm_detach(exchange);
    return EXIT_SUCCESS;
}

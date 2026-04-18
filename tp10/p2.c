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
    int table[TABLE_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};
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
        int minimum_index = 0;

        for (int index = 1; index < TABLE_SIZE; index++) {
            if (table[index] < table[minimum_index]) {
                minimum_index = index;
            }
        }
        exchange->second = table[minimum_index];
        if (sem_v(semaphores, EXCH_P2) < 0) {
            die("sem_v failed");
        }
        if (sem_p(semaphores, EXCH_P0_TO_P2) < 0) {
            die("sem_p failed");
        }
        if (exchange->third == -1) {
            break;
        }
        table[minimum_index] = exchange->second;
    }
    print_table("P2", table);
    shm_detach(exchange);
    return EXIT_SUCCESS;
}

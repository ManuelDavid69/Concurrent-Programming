#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int item_count = 20;
    int semaphores;
    int memory;
    Buffer *buffer;
    bool last;
    int index;

    if (argc >= 2 && (!parse_int(argv[1], &item_count) || item_count < 0 || item_count > 1000000)) {
        fprintf(stderr, "usage: %s [items]\n", argv[0]);
        return EXIT_FAILURE;
    }
    semaphores = sem_open(TP10_PC_SEM_KEY, PC_SEM_COUNT);
    memory = shm_open(TP10_PC_SHM_KEY, 0);
    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    buffer = shm_attach(memory);
    if (buffer == NULL) {
        die("cannot attach the buffer");
    }

    if (sem_p(semaphores, PC_MUTEX) < 0) {
        die("sem_p failed");
    }
    buffer->producers_started++;
    buffer->producers_active++;
    if (sem_v(semaphores, PC_MUTEX) < 0) {
        die("sem_v failed");
    }
    printf("Producteur %ld: démarre (%d objets)\n", (long)getpid(), item_count);
    fflush(stdout);

    for (index = 0; index < item_count; index++) {
        int value = (int)(getpid() % 1000) * 1000 + index;
        if (sem_p(semaphores, PC_EMPTY) < 0) {
            die("sem_p empty failed");
        }
        if (sem_p(semaphores, PC_MUTEX) < 0) {
            die("sem_p mutex failed");
        }
        buffer->items[buffer->tail] = value;
        buffer->tail = (buffer->tail + 1) % buffer->capacity;
        buffer->count++;
        if (sem_v(semaphores, PC_MUTEX) < 0) {
            die("sem_v mutex failed");
        }
        if (sem_v(semaphores, PC_FULL) < 0) {
            die("sem_v full failed");
        }
        printf("Producteur %ld: dépose %d\n", (long)getpid(), value);
        fflush(stdout);
    }

    if (sem_p(semaphores, PC_MUTEX) < 0) {
        die("sem_p failed");
    }
    buffer->producers_active--;
    last = buffer->producers_started == buffer->producers_expected
        && buffer->producers_active == 0;
    if (last) {
        buffer->finished = 1;
    }
    if (sem_v(semaphores, PC_MUTEX) < 0) {
        die("sem_v failed");
    }
    if (last) {
        for (index = 0; index < 1024; index++) {
            sem_v(semaphores, PC_FULL);
        }
    }
    printf("Producteur %ld: terminé\n", (long)getpid());
    shm_detach(buffer);
    return EXIT_SUCCESS;
}

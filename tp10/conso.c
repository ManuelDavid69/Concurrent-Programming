#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int semaphores;
    int memory;
    Buffer *buffer;
    int consumed = 0;

    semaphores = sem_open(TP10_PC_SEM_KEY, PC_SEM_COUNT);
    memory = shm_open(TP10_PC_SHM_KEY, 0);
    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    buffer = shm_attach(memory);
    if (buffer == NULL) {
        die("cannot attach the buffer");
    }

    for (;;) {
        int value;

        if (sem_p(semaphores, PC_FULL) < 0) {
            die("sem_p full failed");
        }
        if (sem_p(semaphores, PC_MUTEX) < 0) {
            die("sem_p mutex failed");
        }
        if (buffer->count == 0) {
            int finished = buffer->finished;
            if (sem_v(semaphores, PC_MUTEX) < 0) {
                die("sem_v mutex failed");
            }
            if (finished) {
                break;
            }
            continue;
        }
        value = buffer->items[buffer->head];
        buffer->head = (buffer->head + 1) % buffer->capacity;
        buffer->count--;
        if (sem_v(semaphores, PC_MUTEX) < 0) {
            die("sem_v mutex failed");
        }
        if (sem_v(semaphores, PC_EMPTY) < 0) {
            die("sem_v empty failed");
        }
        printf("Consommateur %ld: récupère %d\n", (long)getpid(), value);
        fflush(stdout);
        consumed++;
    }
    printf("Consommateur %ld: terminé (%d objets)\n", (long)getpid(), consumed);
    shm_detach(buffer);
    return EXIT_SUCCESS;
}

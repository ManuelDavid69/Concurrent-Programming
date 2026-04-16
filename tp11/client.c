#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static int acquire_slot(int semaphores, CalcShared *shared)
{
    int slot = -1;

    if (sem_p(semaphores, CALC_MUTEX) < 0) {
        die("sem_p failed");
    }
    for (int index = 0; index < CALC_MAX_CLIENTS; index++) {
        if (shared->client_pid[index] == 0) {
            shared->client_pid[index] = (int)getpid();
            slot = index;
            break;
        }
    }
    if (sem_v(semaphores, CALC_MUTEX) < 0) {
        die("sem_v failed");
    }
    return slot;
}

static void release_slot(int semaphores, CalcShared *shared, int slot)
{
    if (sem_p(semaphores, CALC_MUTEX) < 0) {
        die("sem_p failed");
    }
    shared->client_pid[slot] = 0;
    if (sem_v(semaphores, CALC_MUTEX) < 0) {
        die("sem_v failed");
    }
}

int main(int argc, char **argv)
{
    int request_count = 3;
    int semaphores;
    int memory;
    CalcShared *shared;
    int slot;
    unsigned int seed;
    static const char operators[] = {'+', '-', '*', '/'};

    if (argc >= 2 && (!parse_int(argv[1], &request_count) || request_count < 1
                      || request_count > 100)) {
        fprintf(stderr, "usage: %s [requests]\n", argv[0]);
        return EXIT_FAILURE;
    }
    semaphores = sem_open(TP11_CALC_SEM_KEY, CALC_SEM_COUNT);
    memory = shm_open(TP11_CALC_SHM_KEY, sizeof(CalcShared));
    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    shared = shm_attach(memory);
    if (shared == NULL) {
        die("cannot attach the calculator state");
    }
    slot = acquire_slot(semaphores, shared);
    if (slot < 0) {
        die("no free client slot");
    }
    {
        const char *text = getenv("CALC_SEED");
        int parsed;
        seed = text != NULL && parse_int(text, &parsed)
            ? (unsigned int)parsed + (unsigned int)slot * 7919U
            : (unsigned int)(time(NULL) ^ getpid());
    }
    srand(seed);

    for (int request_index = 0; request_index < request_count; request_index++) {
        CalcRequest request;
        CalcResponse response;

        request.client_id = slot;
        request.left = rand_range(0, 99);
        request.right = rand_range(1, 99);
        request.operator = operators[rand_range(0, 3)];

        if (sem_p(semaphores, CALC_SLOTS) < 0) {
            die("sem_p slots failed");
        }
        if (sem_p(semaphores, CALC_MUTEX) < 0) {
            die("sem_p mutex failed");
        }
        shared->queue[shared->tail] = request;
        shared->tail = (shared->tail + 1) % CALC_QUEUE_CAPACITY;
        shared->count++;
        if (sem_v(semaphores, CALC_MUTEX) < 0 || sem_v(semaphores, CALC_ITEMS) < 0) {
            die("sem_v failed");
        }
        if (sem_p(semaphores, CALC_RESULT_BASE + slot) < 0) {
            die("sem_p result failed");
        }
        if (sem_p(semaphores, CALC_MUTEX) < 0) {
            die("sem_p mutex failed");
        }
        response = shared->results[slot];
        if (sem_v(semaphores, CALC_MUTEX) < 0) {
            die("sem_v failed");
        }
        printf("Client %d (%ld): %d %c %d = %d\n", slot, (long)getpid(), response.left,
               response.operator, response.right, response.result);
        fflush(stdout);
    }
    release_slot(semaphores, shared, slot);
    shm_detach(shared);
    return EXIT_SUCCESS;
}

#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t stopping = 0;

static void on_sigint(int signal_number)
{
    (void)signal_number;
    stopping = 1;
}

static int evaluate(const CalcRequest *request, CalcResponse *response)
{
    response->left = request->left;
    response->right = request->right;
    response->operator = request->operator;
    switch (request->operator) {
    case '+':
        response->result = request->left + request->right;
        return 0;
    case '-':
        response->result = request->left - request->right;
        return 0;
    case '*':
        response->result = request->left * request->right;
        return 0;
    case '/':
        if (request->right == 0) {
            return -1;
        }
        response->result = request->left / request->right;
        return 0;
    default:
        return -1;
    }
}

int main(void)
{
    int semaphores;
    int memory;
    CalcShared *shared;
    struct sigaction action;

    action.sa_handler = on_sigint;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    semaphores = sem_open(TP11_CALC_SEM_KEY, CALC_SEM_COUNT);
    memory = shm_open(TP11_CALC_SHM_KEY, sizeof(CalcShared));
    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    shared = shm_attach(memory);
    if (shared == NULL) {
        die("cannot attach the calculator state");
    }
    printf("Serveur %ld: prêt\n", (long)getpid());
    fflush(stdout);

    while (!stopping) {
        CalcRequest request;
        CalcResponse response;

        if (sem_p_interruptible(semaphores, CALC_ITEMS) < 0) {
            if (errno == EINTR && stopping) {
                break;
            }
            die("sem_p items failed");
        }
        if (sem_p(semaphores, CALC_MUTEX) < 0) {
            die("sem_p mutex failed");
        }
        request = shared->queue[shared->head];
        shared->head = (shared->head + 1) % CALC_QUEUE_CAPACITY;
        shared->count--;
        if (sem_v(semaphores, CALC_MUTEX) < 0 || sem_v(semaphores, CALC_SLOTS) < 0) {
            die("sem_v failed");
        }
        if (request.client_id < 0 || request.client_id >= CALC_MAX_CLIENTS) {
            fprintf(stderr, "invalid client id %d\n", request.client_id);
            continue;
        }
        if (evaluate(&request, &response) != 0) {
            response.left = request.left;
            response.right = request.right;
            response.operator = request.operator;
            response.result = 0;
        }
        if (sem_p(semaphores, CALC_MUTEX) < 0) {
            die("sem_p mutex failed");
        }
        shared->results[request.client_id] = response;
        if (sem_v(semaphores, CALC_MUTEX) < 0) {
            die("sem_v failed");
        }
        if (sem_v(semaphores, CALC_RESULT_BASE + request.client_id) < 0) {
            die("sem_v result failed");
        }
        printf("Serveur %ld: client %d, %d %c %d = %d\n", (long)getpid(),
               request.client_id, response.left, response.operator, response.right,
               response.result);
        fflush(stdout);
    }
    printf("Serveur %ld: arrêt\n", (long)getpid());
    shm_detach(shared);
    return EXIT_SUCCESS;
}

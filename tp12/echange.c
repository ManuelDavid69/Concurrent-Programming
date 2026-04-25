#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define TABLE_SIZE 8

typedef struct {
    int *table;
    int identifier;
} ThreadArgument;

static sem_t p1_ready;
static sem_t p2_ready;
static sem_t p0_to_p1;
static sem_t p0_to_p2;
static int cell_first;
static int cell_second;
static int cell_third;

static void print_table(const char *label, const int *table)
{
    char line[256];
    int offset = 0;

    offset += snprintf(line + offset, sizeof(line) - (size_t)offset, "%s:", label);
    for (int index = 0; index < TABLE_SIZE; index++) {
        offset += snprintf(line + offset, sizeof(line) - (size_t)offset, " %d", table[index]);
    }
    printf("%s\n", line);
}

static void *worker_p1(void *argument)
{
    ThreadArgument *thread = argument;

    for (;;) {
        int maximum_index = 0;
        for (int index = 1; index < TABLE_SIZE; index++) {
            if (thread->table[index] > thread->table[maximum_index]) {
                maximum_index = index;
            }
        }
        cell_first = thread->table[maximum_index];
        sem_post(&p1_ready);
        sem_wait(&p0_to_p1);
        if (cell_third == -1) {
            break;
        }
        thread->table[maximum_index] = cell_first;
    }
    print_table("T1", thread->table);
    return NULL;
}

static void *worker_p2(void *argument)
{
    ThreadArgument *thread = argument;

    for (;;) {
        int minimum_index = 0;
        for (int index = 1; index < TABLE_SIZE; index++) {
            if (thread->table[index] < thread->table[minimum_index]) {
                minimum_index = index;
            }
        }
        cell_second = thread->table[minimum_index];
        sem_post(&p2_ready);
        sem_wait(&p0_to_p2);
        if (cell_third == -1) {
            break;
        }
        thread->table[minimum_index] = cell_second;
    }
    print_table("T2", thread->table);
    return NULL;
}

static void *worker_p0(void *argument)
{
    int exchanges = 0;

    (void)argument;
    for (;;) {
        sem_wait(&p1_ready);
        sem_wait(&p2_ready);
        if (cell_first > cell_second) {
            int temporary = cell_first;
            cell_first = cell_second;
            cell_second = temporary;
            cell_third = 0;
            exchanges++;
        } else {
            cell_third = -1;
        }
        sem_post(&p0_to_p1);
        sem_post(&p0_to_p2);
        if (cell_third == -1) {
            break;
        }
    }
    printf("T0: %d échange(s), les tableaux sont ordonnés\n", exchanges);
    return NULL;
}

int main(void)
{
    int table_one[TABLE_SIZE] = {20, 30, 40, 50, 60, 70, 80, 90};
    int table_two[TABLE_SIZE] = {1, 2, 3, 4, 5, 6, 7, 8};
    ThreadArgument arguments[2] = {{table_one, 1}, {table_two, 2}};
    pthread_t t0;
    pthread_t t1;
    pthread_t t2;

    sem_init(&p1_ready, 0, 0);
    sem_init(&p2_ready, 0, 0);
    sem_init(&p0_to_p1, 0, 0);
    sem_init(&p0_to_p2, 0, 0);
    pthread_create(&t0, NULL, worker_p0, NULL);
    pthread_create(&t1, NULL, worker_p1, &arguments[0]);
    pthread_create(&t2, NULL, worker_p2, &arguments[1]);
    pthread_join(t0, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    sem_destroy(&p0_to_p2);
    sem_destroy(&p0_to_p1);
    sem_destroy(&p2_ready);
    sem_destroy(&p1_ready);
    return EXIT_SUCCESS;
}

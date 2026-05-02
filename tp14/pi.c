#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    long samples;
    long hits;
    uint64_t state;
} PiArgument;

static uint64_t next_random(uint64_t *state)
{
    uint64_t value = *state;

    value ^= value >> 12;
    value ^= value << 25;
    value ^= value >> 27;
    *state = value;
    return value * 2685821657736338717ULL;
}

static double unit_random(uint64_t *state)
{
    return (double)(next_random(state) >> 11) * (1.0 / 9007199254740992.0);
}

static long count_hits(long samples, uint64_t seed)
{
    uint64_t state = seed;
    long hits = 0;

    for (long index = 0; index < samples; index++) {
        double x = unit_random(&state);
        double y = unit_random(&state);
        if (x * x + y * y <= 1.0) {
            hits++;
        }
    }
    return hits;
}

static void *worker(void *argument)
{
    PiArgument *pi = argument;

    pi->hits = count_hits(pi->samples, pi->state);
    return NULL;
}

static double now_seconds(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (double)time.tv_sec + (double)time.tv_nsec / 1e9;
}

static double estimate(long samples, int threads, double *elapsed)
{
    pthread_t *workers;
    PiArgument *arguments;
    long base;
    long remainder;
    long total_hits = 0;
    double start;
    uint64_t seed = (uint64_t)time(NULL) ^ ((uint64_t)getpid() << 32);

    if (threads > (int)samples) {
        threads = (int)samples;
    }
    workers = xmalloc((size_t)threads * sizeof(*workers));
    arguments = xmalloc((size_t)threads * sizeof(*arguments));
    base = samples / threads;
    remainder = samples % threads;
    start = now_seconds();
    for (int index = 0; index < threads; index++) {
        arguments[index].samples = base + (index < remainder ? 1 : 0);
        arguments[index].hits = 0;
        arguments[index].state = seed + (uint64_t)index * 0x9E3779B97F4A7C15ULL;
        if (pthread_create(&workers[index], NULL, worker, &arguments[index]) != 0) {
            die("pthread_create failed");
        }
    }
    for (int index = 0; index < threads; index++) {
        pthread_join(workers[index], NULL);
        total_hits += arguments[index].hits;
    }
    *elapsed = now_seconds() - start;
    free(arguments);
    free(workers);
    return 4.0 * (double)total_hits / (double)samples;
}

int main(int argc, char **argv)
{
    long samples = 1000000;
    int threads = 4;
    int compare = 0;
    double elapsed;
    double pi;

    if (argc >= 2 && strcmp(argv[1], "--compare") == 0) {
        compare = 1;
        argv++;
        argc--;
    }
    if (argc >= 2 && (!parse_long(argv[1], &samples) || samples <= 0)) {
        fprintf(stderr, "usage: %s [--compare] [samples] [threads]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 3 && (!parse_int(argv[2], &threads) || threads < 1 || threads > 256)) {
        fprintf(stderr, "invalid number of threads\n");
        return EXIT_FAILURE;
    }
    if (compare) {
        double single_elapsed;
        double multi_elapsed;

        pi = estimate(samples, 1, &single_elapsed);
        printf("Mono-thread: Pi = %.6f (N=%ld, %.3f s)\n", pi, samples, single_elapsed);
        pi = estimate(samples, threads, &multi_elapsed);
        printf("Multi-thread: Pi = %.6f (N=%ld, %d threads, %.3f s)\n", pi, samples,
               threads, multi_elapsed);
        if (multi_elapsed > 0.0) {
            printf("Accélération: %.2fx\n", single_elapsed / multi_elapsed);
        }
    } else {
        pi = estimate(samples, threads, &elapsed);
        printf("Pi = %.6f (N=%ld, %d thread(s), %.3f s)\n", pi, samples, threads, elapsed);
    }
    return EXIT_SUCCESS;
}

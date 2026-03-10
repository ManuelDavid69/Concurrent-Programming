#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static void close_pair(int pair[2])
{
    close(pair[0]);
    close(pair[1]);
}

static void filter_process(int input, int even_write, int odd_write)
{
    int value;

    while (read_all(input, &value, sizeof(value)) == 0) {
        if (value == -1) {
            break;
        }
        if (value % 2 == 0) {
            if (write_all(even_write, &value, sizeof(value)) != 0) {
                break;
            }
        } else {
            if (write_all(odd_write, &value, sizeof(value)) != 0) {
                break;
            }
        }
    }
    value = -1;
    write_all(even_write, &value, sizeof(value));
    write_all(odd_write, &value, sizeof(value));
    close(input);
    close(even_write);
    close(odd_write);
    _exit(EXIT_SUCCESS);
}

static void summer_process(int input, int output)
{
    int value;
    int sum = 0;

    while (read_all(input, &value, sizeof(value)) == 0) {
        if (value == -1) {
            break;
        }
        sum += value;
    }
    write_all(output, &sum, sizeof(sum));
    close(input);
    close(output);
    _exit(EXIT_SUCCESS);
}

static void wait_children(pid_t *children, int count)
{
    int index;

    for (index = 0; index < count; index++) {
        int status;
        if (waitpid(children[index], &status, 0) < 0) {
            die("waitpid failed");
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            die("a child process failed");
        }
    }
}

int main(int argc, char **argv)
{
    int count = 20;
    long seed = -1;
    bool verbose = false;
    int generator_filter[2];
    int even_pipe[2];
    int odd_pipe[2];
    int even_sum[2];
    int odd_sum[2];
    pid_t children[3];
    int index;

    if (argc >= 2) {
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0) {
            verbose = true;
            argv++;
            argc--;
        }
    }
    if (argc >= 2 && (!parse_int(argv[1], &count) || count < 0 || count > 1000000)) {
        fprintf(stderr, "usage: %s [--verbose] [N] [seed]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 3 && !parse_long(argv[2], &seed)) {
        fprintf(stderr, "invalid seed: %s\n", argv[2]);
        return EXIT_FAILURE;
    }
    if (seed >= 0) {
        srand((unsigned int)seed);
    } else {
        pc_seed();
    }

    if (pipe(generator_filter) < 0 || pipe(even_pipe) < 0 || pipe(odd_pipe) < 0
        || pipe(even_sum) < 0 || pipe(odd_sum) < 0) {
        die("pipe failed");
    }

    children[0] = xfork();
    if (children[0] == 0) {
        close(generator_filter[1]);
        close(even_pipe[0]);
        close(odd_pipe[0]);
        close_pair(even_sum);
        close_pair(odd_sum);
        filter_process(generator_filter[0], even_pipe[1], odd_pipe[1]);
    }

    children[1] = xfork();
    if (children[1] == 0) {
        close_pair(generator_filter);
        close(even_pipe[1]);
        close_pair(odd_pipe);
        close(even_sum[0]);
        close_pair(odd_sum);
        summer_process(even_pipe[0], even_sum[1]);
    }

    children[2] = xfork();
    if (children[2] == 0) {
        close_pair(generator_filter);
        close_pair(even_pipe);
        close(odd_pipe[1]);
        close_pair(even_sum);
        close(odd_sum[0]);
        summer_process(odd_pipe[0], odd_sum[1]);
    }

    close(generator_filter[0]);
    close(even_pipe[1]);
    close(odd_pipe[1]);
    close(even_sum[1]);
    close(odd_sum[1]);

    if (verbose) {
        printf("Nombres générés:");
    }
    for (index = 0; index < count; index++) {
        int value = rand_range(0, 999);
        if (verbose) {
            printf(" %d", value);
        }
        if (write_all(generator_filter[1], &value, sizeof(value)) != 0) {
            die("cannot send a number to the filter");
        }
    }
    if (verbose) {
        printf("\n");
    }
    {
        int sentinel = -1;
        write_all(generator_filter[1], &sentinel, sizeof(sentinel));
    }
    close(generator_filter[1]);

    {
        int even_total = 0;
        int odd_total = 0;
        if (read_all(even_sum[0], &even_total, sizeof(even_total)) != 0
            || read_all(odd_sum[0], &odd_total, sizeof(odd_total)) != 0) {
            die("cannot read the partial sums");
        }
        printf("Somme des pairs: %d\n", even_total);
        printf("Somme des impairs: %d\n", odd_total);
        printf("Somme totale: %d\n", even_total + odd_total);
    }
    close(even_sum[0]);
    close(odd_sum[0]);
    wait_children(children, 3);
    return EXIT_SUCCESS;
}

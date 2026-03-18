#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    int value;
    pid_t pid;
    int rank;
} Candidate;

static void close_unused(int (*pipes)[2], int count, int read_index, int write_index)
{
    int index;

    for (index = 0; index < count; index++) {
        if (index != read_index) {
            close(pipes[index][0]);
        }
        if (index != write_index) {
            close(pipes[index][1]);
        }
    }
}

static void process_ring(int (*pipes)[2], int count, int rank)
{
    int previous = (rank - 1 + count) % count;
    int next = rank;
    Candidate mine;
    Candidate received;

    close_unused(pipes, count, previous, next);
    pc_seed();
    mine.value = rand_range(0, 9999);
    mine.pid = getpid();
    mine.rank = rank;
    printf("processus pid %ld numéro %d val = %d\n", (long)mine.pid, mine.rank, mine.value);
    fflush(stdout);

    if (rank == 0) {
        if (write_all(pipes[next][1], &mine, sizeof(mine)) != 0) {
            die("cannot send the first candidate");
        }
        if (read_all(pipes[previous][0], &received, sizeof(received)) != 0) {
            die("cannot receive the final candidate");
        }
        printf("Le plus grand nombre = %d - pid = %ld - Numéro %d\n",
               received.value, (long)received.pid, received.rank);
    } else {
        if (read_all(pipes[previous][0], &received, sizeof(received)) != 0) {
            die("process %d cannot read from its predecessor", rank);
        }
        if (received.value > mine.value) {
            mine = received;
        }
        if (write_all(pipes[next][1], &mine, sizeof(mine)) != 0) {
            die("process %d cannot write to its successor", rank);
        }
    }
    close(pipes[previous][0]);
    close(pipes[next][1]);
}

int main(int argc, char **argv)
{
    int count;
    int (*pipes)[2];
    pid_t *children;
    int index;

    if (argc != 2 || !parse_int(argv[1], &count) || count < 1 || count > 256) {
        fprintf(stderr, "usage: %s N (1 <= N <= 256)\n", argv[0]);
        return EXIT_FAILURE;
    }
    pipes = xmalloc((size_t)count * sizeof(*pipes));
    children = xmalloc((size_t)count * sizeof(*children));
    for (index = 0; index < count; index++) {
        if (pipe(pipes[index]) < 0) {
            die("pipe failed");
        }
    }
    for (index = 1; index < count; index++) {
        pid_t child = xfork();
        if (child == 0) {
            free(children);
            process_ring(pipes, count, index);
            free(pipes);
            _exit(EXIT_SUCCESS);
        }
        children[index] = child;
    }
    process_ring(pipes, count, 0);
    free(pipes);
    for (index = 1; index < count; index++) {
        int status;
        if (waitpid(children[index], &status, 0) < 0) {
            die("waitpid failed");
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            die("ring process %d failed", index);
        }
    }
    free(children);
    return EXIT_SUCCESS;
}

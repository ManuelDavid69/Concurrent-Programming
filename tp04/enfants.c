#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CHILDREN 4
#define FIRST 1
#define LAST 200
#define PER_CHILD 50

static void print_range(int child_index)
{
    int first = FIRST + child_index * PER_CHILD;
    int last = first + PER_CHILD - 1;
    int value;

    for (value = first; value <= last; value++) {
        printf("%d\n", value);
        fflush(stdout);
    }
}

/*
 * Variant 1: the four children run concurrently, so their outputs interleave.
 */
static int run_parallel(void)
{
    int child_index;

    for (child_index = 0; child_index < CHILDREN; child_index++) {
        pid_t child = xfork();
        if (child == 0) {
            print_range(child_index);
            exit(EXIT_SUCCESS);
        }
    }
    while (wait(NULL) > 0) {
        /* Wait for all four children. */
    }
    return EXIT_SUCCESS;
}

/*
 * Variant 2: all four children exist, but each one waits for a token from the
 * parent.  The parent sends the token to child i, waits for it, then releases
 * child i + 1.  The output is therefore exactly 1, 2, ..., 200.
 */
static int run_ordered(void)
{
    int tokens[CHILDREN][2];
    pid_t children[CHILDREN];
    int child_index;

    for (child_index = 0; child_index < CHILDREN; child_index++) {
        if (pipe(tokens[child_index]) < 0) {
            die("pipe failed");
        }
    }
    for (child_index = 0; child_index < CHILDREN; child_index++) {
        pid_t child = xfork();
        if (child == 0) {
            char token;
            int other;
            for (other = 0; other < CHILDREN; other++) {
                if (other != child_index) {
                    close(tokens[other][0]);
                }
                close(tokens[other][1]);
            }
            if (read_all(tokens[child_index][0], &token, 1) != 0) {
                _exit(EXIT_FAILURE);
            }
            close(tokens[child_index][0]);
            print_range(child_index);
            _exit(EXIT_SUCCESS);
        }
        children[child_index] = child;
    }
    for (child_index = 0; child_index < CHILDREN; child_index++) {
        char token = 'x';
        int status;
        close(tokens[child_index][0]);
        if (write_all(tokens[child_index][1], &token, 1) != 0) {
            die("cannot release child %d", child_index);
        }
        close(tokens[child_index][1]);
        if (waitpid(children[child_index], &status, 0) < 0) {
            die("waitpid failed");
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            die("child %d failed", child_index);
        }
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
    if (argc == 1) {
        return run_parallel();
    }
    if (argc == 2 && (strcmp(argv[1], "--ordered") == 0 || strcmp(argv[1], "-o") == 0)) {
        return run_ordered();
    }
    fprintf(stderr, "usage: %s [--ordered]\n", argv[0]);
    return EXIT_FAILURE;
}

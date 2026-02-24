#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static void run_child(int index)
{
    printf("Fils %d: pid=%ld ppid=%ld, je dors %d seconde(s)\n",
           index, (long)getpid(), (long)getppid(), 2 * index);
    fflush(stdout);
    pc_sleep(2.0 * index);
    printf("Fils %d: je reprends l'exécution et je termine avec le code %d\n", index, index);
    fflush(stdout);
    exit(index);
}

int main(int argc, char **argv)
{
    int count;
    pid_t *children;
    int finished = 0;
    int index;

    if (argc != 2 || !parse_int(argv[1], &count) || count <= 0 || count > 100) {
        fprintf(stderr, "usage: %s N (1 <= N <= 100)\n", argv[0]);
        return EXIT_FAILURE;
    }
    children = xmalloc((size_t)count * sizeof(*children));

    for (index = 0; index < count; index++) {
        pid_t child = xfork();
        if (child == 0) {
            free(children);
            run_child(index);
        }
        children[index] = child;
    }

    printf("Père: les %d fils sont créés, j'attends leur terminaison\n", count);
    fflush(stdout);
    while (finished < count) {
        int status;
        pid_t child = waitpid(-1, &status, 0);

        if (child < 0) {
            break;
        }
        if (WIFEXITED(status)) {
            printf("Père: le fils %ld est terminé avec le code %d\n",
                   (long)child, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Père: le fils %ld a été tué par le signal %d\n",
                   (long)child, WTERMSIG(status));
        }
        fflush(stdout);
        finished++;
    }
    free(children);
    return EXIT_SUCCESS;
}

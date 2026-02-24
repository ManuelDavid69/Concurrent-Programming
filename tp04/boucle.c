#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * fork() is called three times in a loop, without any condition on the call
 * or on the display.  Every process therefore continues the loop, which
 * produces the complete binary tree of processes (2^3 processes).
 */
int main(void)
{
    int iteration;

    for (iteration = 0; iteration < 3; iteration++) {
        pid_t result = xfork();
        printf("%d: Je suis le processus %ld, mon père est le processus %ld, fork a retourné %ld\n",
               iteration, (long)getpid(), (long)getppid(), (long)result);
        fflush(stdout);
    }
    while (wait(NULL) > 0) {
        /* Reap every descendant so that the shell prompt comes back cleanly. */
    }
    return EXIT_SUCCESS;
}

#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t child = xfork();
    int iteration;

    if (child == 0) {
        for (;;) {
            printf("Fils %ld: affichage périodique\n", (long)getpid());
            fflush(stdout);
            pc_sleep(1);
        }
    }
    for (iteration = 1; iteration <= 3; iteration++) {
        printf("Père: itération %d\n", iteration);
        fflush(stdout);
        pc_sleep(1);
    }
    printf("Père: envoi de SIGKILL au fils %ld\n", (long)child);
    fflush(stdout);
    if (kill(child, SIGKILL) < 0) {
        die("kill failed");
    }
    waitpid(child, NULL, 0);
    printf("Père: le fils a été tué\n");
    return EXIT_SUCCESS;
}

#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void on_sigint(int signal_number)
{
    static const char message[] = "Fils: SIGINT intercepté, je continue\n";
    (void)signal_number;
    {
        ssize_t written = write(STDOUT_FILENO, message, sizeof(message) - 1);
        (void)written;
    }
}

int main(void)
{
    pid_t child = xfork();
    int iteration;

    if (child == 0) {
        struct sigaction action;
        action.sa_handler = on_sigint;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        if (sigaction(SIGINT, &action, NULL) < 0) {
            _exit(EXIT_FAILURE);
        }
        printf("Fils pid=%ld: prêt, j'intercepte SIGINT\n", (long)getpid());
        fflush(stdout);
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
    printf("Père: fin, le fils %ld continue tout seul\n", (long)child);
    fflush(stdout);
    return EXIT_SUCCESS;
}

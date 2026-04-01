#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void on_sigint(int signal_number)
{
    static const char message[] = "Signal SIGINT reçu, arrêt du programme\n";
    (void)signal_number;
    ssize_t written = write(STDOUT_FILENO, message, sizeof(message) - 1);
    (void)written;
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    struct sigaction action;

    action.sa_handler = on_sigint;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, NULL) < 0) {
        die("sigaction failed");
    }
    for (;;) {
        printf("En cours d'exécution...\n");
        fflush(stdout);
        pc_sleep(1);
    }
}

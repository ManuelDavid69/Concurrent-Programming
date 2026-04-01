#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t running = 1;

static void on_signal(int signal_number)
{
    static const char usr1[] = "Fils: SIGUSR1 reçu, affichage\n";
    static const char usr2[] = "Fils: SIGUSR2 reçu, fin du traitement\n";

    if (signal_number == SIGUSR1) {
        ssize_t written = write(STDOUT_FILENO, usr1, sizeof(usr1) - 1);
        (void)written;
    } else if (signal_number == SIGUSR2) {
        ssize_t written = write(STDOUT_FILENO, usr2, sizeof(usr2) - 1);
        (void)written;
        running = 0;
    }
}

int main(void)
{
    int ready[2];
    pid_t child;
    int iteration;

    if (pipe(ready) < 0) {
        die("pipe failed");
    }
    child = xfork();
    if (child == 0) {
        struct sigaction action;
        sigset_t blocked;
        sigset_t empty;

        close(ready[0]);
        action.sa_handler = on_signal;
        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;
        sigaction(SIGUSR1, &action, NULL);
        sigaction(SIGUSR2, &action, NULL);
        sigemptyset(&blocked);
        sigaddset(&blocked, SIGUSR1);
        sigaddset(&blocked, SIGUSR2);
        sigprocmask(SIG_BLOCK, &blocked, NULL);
        sigemptyset(&empty);
        printf("Fils pid=%ld: prêt pour SIGUSR1/SIGUSR2\n", (long)getpid());
        fflush(stdout);
        {
            char token = 'R';
            write_all(ready[1], &token, 1);
        }
        close(ready[1]);
        while (running) {
            sigsuspend(&empty);
        }
        printf("Fils: terminé\n");
        fflush(stdout);
        _exit(EXIT_SUCCESS);
    }

    close(ready[1]);
    {
        char token;
        if (read_all(ready[0], &token, 1) != 0) {
            die("the child did not become ready");
        }
    }
    close(ready[0]);

    for (iteration = 1; iteration <= 5; iteration++) {
        printf("Père: itération %d\n", iteration);
        fflush(stdout);
        if (iteration == 3 || iteration == 5) {
            printf("Père: envoi de SIGUSR1\n");
            fflush(stdout);
            kill(child, SIGUSR1);
        }
        pc_sleep(1);
    }
    printf("Père: envoi de SIGUSR2 (fin)\n");
    fflush(stdout);
    kill(child, SIGUSR2);
    waitpid(child, NULL, 0);
    printf("Père: le fils est terminé\n");
    return EXIT_SUCCESS;
}

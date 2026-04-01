#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t timed_out = 0;

static void on_alarm(int signal_number)
{
    (void)signal_number;
    timed_out = 1;
}

static int timeout_seconds(void)
{
    const char *text = getenv("PC_TIMEOUT_SECONDS");
    int seconds;

    if (text != NULL && parse_int(text, &seconds) && seconds > 0 && seconds <= 60) {
        return seconds;
    }
    return 5;
}

int main(void)
{
    struct sigaction action;
    char line[128];
    int seconds = timeout_seconds();

    action.sa_handler = on_alarm;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, NULL) < 0) {
        die("sigaction failed");
    }

    printf("Entrez un entier en moins de %d secondes\n", seconds);
    fflush(stdout);
    alarm((unsigned int)seconds);

    for (;;) {
        int value;

        printf("Svp un entier : ");
        fflush(stdout);
        errno = 0;
        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (timed_out) {
                break;
            }
            if (errno == EINTR) {
                continue;
            }
            printf("\nFin de l'entrée\n");
            return EXIT_FAILURE;
        }
        line[strcspn(line, "\r\n")] = '\0';
        if (parse_int(line, &value)) {
            alarm(0);
            printf("Ok merci !!\n");
            return EXIT_SUCCESS;
        }
        if (timed_out) {
            break;
        }
    }
    printf("Trop tard !!\n");
    return EXIT_FAILURE;
}

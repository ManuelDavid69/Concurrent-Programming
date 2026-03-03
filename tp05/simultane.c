#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Equivalent of: du -sh /usr/share & ls -l & ps
 * The three commands are started before any of them is awaited.
 */
int main(void)
{
    char *const commands[][4] = {
        {"du", "-sh", "/usr/share", NULL},
        {"ls", "-l", NULL, NULL},
        {"ps", NULL, NULL, NULL},
    };
    const char *labels[] = {"du -sh /usr/share", "ls -l", "ps"};
    pid_t children[3];
    int index;
    int failed = 0;

    for (index = 0; index < 3; index++) {
        pid_t child = xfork();
        if (child == 0) {
            execvp(commands[index][0], commands[index]);
            die("execvp %s failed", commands[index][0]);
        }
        children[index] = child;
        printf("Lancé: %s (pid %ld)\n", labels[index], (long)child);
        fflush(stdout);
    }
    for (index = 0; index < 3; index++) {
        int status;
        if (waitpid(children[index], &status, 0) < 0) {
            die("waitpid failed");
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Terminé: %s\n", labels[index]);
        } else {
            printf("Échec: %s\n", labels[index]);
            failed = 1;
        }
    }
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

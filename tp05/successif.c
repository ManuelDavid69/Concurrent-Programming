#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Équivalent de : du -sh /usr/share ; ls -l ; ps
 * Chaque commande est attendue avant que la suivante ne soit lancée.
 */
int main(void)
{
    const char *path = getenv("SIMULTANE_PATH");
    char first_label[256];
    char *commands[][4] = {
        {"du", "-sh", "/usr/share", NULL},
        {"ls", "-l", NULL, NULL},
        {"ps", NULL, NULL, NULL},
    };
    const char *labels[3];
    int index;
    int failed = 0;

    if (path == NULL || *path == '\0') {
        path = "/usr/share";
    }
    commands[0][2] = (char *)path;
    snprintf(first_label, sizeof(first_label), "du -sh %s", path);
    labels[0] = first_label;
    labels[1] = "ls -l";
    labels[2] = "ps";

    for (index = 0; index < 3; index++) {
        pid_t child = xfork();
        if (child == 0) {
            execvp(commands[index][0], commands[index]);
            die("execvp %s failed", commands[index][0]);
        }
        printf("Lancé: %s (pid %ld)\n", labels[index], (long)child);
        fflush(stdout);
        {
            int status;
            if (waitpid(child, &status, 0) < 0) {
                die("waitpid failed");
            }
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("Terminé: %s\n", labels[index]);
            } else {
                printf("Échec: %s\n", labels[index]);
                failed = 1;
            }
        }
    }
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

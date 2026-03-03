#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_DOWNLOADS 4

typedef struct {
    pid_t pid;
    char *url;
} Running;

static void report_finished(Running *running, int count, int *remaining, int *failed)
{
    int status;
    pid_t child = waitpid(-1, &status, 0);

    if (child < 0) {
        die("waitpid failed");
    }
    for (int index = 0; index < count; index++) {
        if (running[index].pid == child) {
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("Téléchargement terminé: %s\n", running[index].url);
            } else {
                printf("Échec du téléchargement: %s (code %d)\n", running[index].url,
                       WIFEXITED(status) ? WEXITSTATUS(status) : -1);
                *failed = 1;
            }
            running[index].pid = -1;
            free(running[index].url);
            running[index].url = NULL;
            (*remaining)--;
            return;
        }
    }
}

static pid_t start_download(const char *url, const char *destination)
{
    pid_t child = xfork();

    if (child == 0) {
        char *const arguments[] = {
            "wget", "-q", "-P", (char *)destination, (char *)url, NULL
        };
        execvp(arguments[0], arguments);
        _exit(127);
    }
    return child;
}

int main(int argc, char **argv)
{
    const char *destination;
    Running running[MAX_DOWNLOADS] = {{0}};
    int active = 0;
    int failed = 0;
    int index;

    if (argc < 3) {
        fprintf(stderr, "usage: %s url [url ...] destination_directory\n", argv[0]);
        return EXIT_FAILURE;
    }
    destination = argv[argc - 1];
    if (mkdir(destination, 0755) < 0 && errno != EEXIST) {
        fprintf(stderr, "cannot create %s: %s\n", destination, strerror(errno));
        return EXIT_FAILURE;
    }

    for (index = 1; index < argc - 1; index++) {
        int slot = -1;
        pid_t child;

        while (active >= MAX_DOWNLOADS) {
            report_finished(running, MAX_DOWNLOADS, &active, &failed);
        }
        child = start_download(argv[index], destination);
        for (int candidate = 0; candidate < MAX_DOWNLOADS; candidate++) {
            if (running[candidate].pid <= 0) {
                slot = candidate;
                break;
            }
        }
        if (slot < 0) {
            die("internal error: no free download slot");
        }
        running[slot].pid = child;
        running[slot].url = xstrdup(argv[index]);
        active++;
        printf("Début téléchargement: %s (pid %ld)\n", argv[index], (long)child);
        fflush(stdout);
    }
    while (active > 0) {
        report_finished(running, MAX_DOWNLOADS, &active, &failed);
    }
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

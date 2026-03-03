#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_CONVERSIONS 8

typedef struct {
    pid_t pid;
    char *name;
} Running;

static bool is_image(const char *name)
{
    static const char *extensions[] = {
        ".jpg", ".jpeg", ".png", ".ppm", ".bmp", ".gif", ".tif", ".tiff", ".webp"
    };
    const char *dot = strrchr(name, '.');
    size_t index;

    if (dot == NULL || dot == name) {
        return false;
    }
    for (index = 0; index < sizeof(extensions) / sizeof(extensions[0]); index++) {
        if (strcasecmp(dot, extensions[index]) == 0) {
            return true;
        }
    }
    return false;
}

static char *join_path(const char *directory, const char *name)
{
    size_t length = strlen(directory) + strlen(name) + 2;
    char *path = xmalloc(length);

    snprintf(path, length, "%s/%s", directory, name);
    return path;
}

static int start_conversion(const char *source, const char *destination)
{
    pid_t child = xfork();

    if (child == 0) {
        char *const arguments[] = {
            "convert", (char *)source, "-resize", "10%", (char *)destination, NULL
        };
        execvp(arguments[0], arguments);
        _exit(127);
    }
    return (int)child;
}

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
                printf("Fin conversion: %s\n", running[index].name);
            } else {
                printf("Échec conversion: %s (code %d)\n", running[index].name,
                       WIFEXITED(status) ? WEXITSTATUS(status) : -1);
                *failed = 1;
            }
            running[index].pid = -1;
            free(running[index].name);
            running[index].name = NULL;
            (*remaining)--;
            return;
        }
    }
}

int main(int argc, char **argv)
{
    DIR *directory;
    struct dirent *entry;
    char *destination;
    Running running[MAX_CONVERSIONS] = {{0}};
    int active = 0;
    int failed = 0;
    int converted = 0;

    if (argc != 3) {
        fprintf(stderr, "usage: %s source_directory destination_directory\n", argv[0]);
        return EXIT_FAILURE;
    }
    directory = opendir(argv[1]);
    if (directory == NULL) {
        fprintf(stderr, "cannot open %s: %s\n", argv[1], strerror(errno));
        return EXIT_FAILURE;
    }
    if (mkdir(argv[2], 0755) < 0 && errno != EEXIST) {
        fprintf(stderr, "cannot create %s: %s\n", argv[2], strerror(errno));
        closedir(directory);
        return EXIT_FAILURE;
    }

    while ((entry = readdir(directory)) != NULL) {
        char *source;
        pid_t child;

        if (entry->d_name[0] == '.' || !is_image(entry->d_name)) {
            continue;
        }
        source = join_path(argv[1], entry->d_name);
        destination = join_path(argv[2], entry->d_name);

        while (active >= MAX_CONVERSIONS) {
            report_finished(running, MAX_CONVERSIONS, &active, &failed);
        }
        child = start_conversion(source, destination);
        {
            int slot = -1;
            for (int index = 0; index < MAX_CONVERSIONS; index++) {
                if (running[index].pid <= 0) {
                    slot = index;
                    break;
                }
            }
            if (slot < 0) {
                die("internal error: no free conversion slot");
            }
            running[slot].pid = child;
            running[slot].name = xstrdup(entry->d_name);
        }
        active++;
        converted++;
        printf("Début conversion: %s (pid %ld)\n", entry->d_name, (long)child);
        fflush(stdout);
        free(source);
        free(destination);
    }
    closedir(directory);

    while (active > 0) {
        report_finished(running, MAX_CONVERSIONS, &active, &failed);
    }
    printf("%d image(s) traitée(s)\n", converted);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

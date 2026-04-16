#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char *output_directory;
    int semaphores;
    int memory;
    ImageShared *shared;
    int file_count;
    int index;

    if (argc < 3) {
        fprintf(stderr, "usage: %s output_directory image [image ...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    output_directory = argv[1];
    if (mkdir(output_directory, 0755) < 0 && errno != EEXIST) {
        die("cannot create %s", output_directory);
    }
    semaphores = sem_open(TP11_IMG_SEM_KEY, IMG_SEM_COUNT);
    memory = shm_open(TP11_IMG_SHM_KEY, sizeof(ImageShared));
    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    shared = shm_attach(memory);
    if (shared == NULL) {
        die("cannot attach the image state");
    }

    if (sem_p(semaphores, IMG_MUTEX) < 0) {
        die("sem_p failed");
    }
    shared->active_clients++;
    if (sem_v(semaphores, IMG_MUTEX) < 0) {
        die("sem_v failed");
    }

    file_count = argc - 2;
    for (index = 0; index < file_count; index++) {
        const char *path = argv[index + 2];
        if (strlen(path) >= IMG_PATH_MAX) {
            fprintf(stderr, "path too long: %s\n", path);
            continue;
        }
        if (sem_p(semaphores, IMG_SLOTS) < 0) {
            die("sem_p slots failed");
        }
        if (sem_p(semaphores, IMG_MUTEX) < 0) {
            die("sem_p mutex failed");
        }
        snprintf(shared->tasks[shared->tail], IMG_PATH_MAX, "%s", path);
        shared->tail = (shared->tail + 1) % IMG_TASK_SLOTS;
        shared->count++;
        if (sem_v(semaphores, IMG_MUTEX) < 0 || sem_v(semaphores, IMG_ITEMS) < 0) {
            die("sem_v failed");
        }
    }

    if (sem_p(semaphores, IMG_MUTEX) < 0) {
        die("sem_p failed");
    }
    shared->active_clients--;
    if (shared->active_clients == 0) {
        shared->finished = 1;
    }
    if (sem_v(semaphores, IMG_MUTEX) < 0) {
        die("sem_v failed");
    }
    printf("Client image %ld: %d fichier(s) envoyé(s)\n", (long)getpid(), file_count);
    shm_detach(shared);
    return EXIT_SUCCESS;
}

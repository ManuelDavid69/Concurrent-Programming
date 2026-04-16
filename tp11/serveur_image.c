#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "image.h"
#include "state.h"
#include "sysv.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static volatile sig_atomic_t stopping = 0;

static void on_sigint(int signal_number)
{
    (void)signal_number;
    stopping = 1;
}

static char *output_path(const char *directory, const char *source)
{
    const char *base = strrchr(source, '/');
    size_t length;

    base = base == NULL ? source : base + 1;
    length = strlen(directory) + strlen(base) + 2;
    {
        char *path = xmalloc(length);
        snprintf(path, length, "%s/%s", directory, base);
        return path;
    }
}

static int process_image(const char *source, const char *destination)
{
    Image *input = image_load(source);
    Image *blurred;

    if (input == NULL) {
        return -1;
    }
    blurred = image_blur_crop(input);
    if (blurred == NULL) {
        image_free(input);
        return -1;
    }
    if (image_save(destination, blurred) != 0) {
        image_free(input);
        image_free(blurred);
        return -1;
    }
    image_free(input);
    image_free(blurred);
    return 0;
}

int main(int argc, char **argv)
{
    const char *output_directory;
    int drain = 0;
    int semaphores;
    int memory;
    ImageShared *shared;
    struct sigaction action;
    int processed = 0;

    if (argc < 2) {
        fprintf(stderr, "usage: %s output_directory [--drain]\n", argv[0]);
        return EXIT_FAILURE;
    }
    output_directory = argv[1];
    for (int index = 2; index < argc; index++) {
        if (strcmp(argv[index], "--drain") == 0) {
            drain = 1;
        }
    }
    if (mkdir(output_directory, 0755) < 0 && errno != EEXIST) {
        die("cannot create %s", output_directory);
    }
    action.sa_handler = on_sigint;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    semaphores = sem_open(TP11_IMG_SEM_KEY, IMG_SEM_COUNT);
    memory = shm_open(TP11_IMG_SHM_KEY, sizeof(ImageShared));
    if (semaphores < 0 || memory < 0) {
        die("resources not found; run init first");
    }
    shared = shm_attach(memory);
    if (shared == NULL) {
        die("cannot attach the image state");
    }
    printf("Serveur image %ld: prêt%s\n", (long)getpid(), drain ? " (drain)" : "");
    fflush(stdout);

    while (!stopping) {
        char source[IMG_PATH_MAX];
        int got_item = 0;

        if (drain) {
            if (sem_try_p(semaphores, IMG_ITEMS) == 0) {
                got_item = 1;
            } else if (errno != EAGAIN) {
                die("sem_try_p failed");
            } else {
                int finished;
                if (sem_p(semaphores, IMG_MUTEX) < 0) {
                    die("sem_p failed");
                }
                finished = shared->finished && shared->count == 0;
                if (sem_v(semaphores, IMG_MUTEX) < 0) {
                    die("sem_v failed");
                }
                if (finished) {
                    break;
                }
                pc_sleep(0.01);
                continue;
            }
        } else {
            if (sem_p_interruptible(semaphores, IMG_ITEMS) < 0) {
                if (errno == EINTR && stopping) {
                    break;
                }
                die("sem_p items failed");
            }
            got_item = 1;
        }

        if (got_item) {
            char *destination;
            if (sem_p(semaphores, IMG_MUTEX) < 0) {
                die("sem_p mutex failed");
            }
            if (shared->count == 0) {
                int finished = shared->finished;
                if (sem_v(semaphores, IMG_MUTEX) < 0) {
                    die("sem_v failed");
                }
                if (finished) {
                    break;
                }
                continue;
            }
            snprintf(source, sizeof(source), "%s", shared->tasks[shared->head]);
            shared->head = (shared->head + 1) % IMG_TASK_SLOTS;
            shared->count--;
            if (sem_v(semaphores, IMG_MUTEX) < 0 || sem_v(semaphores, IMG_SLOTS) < 0) {
                die("sem_v failed");
            }
            destination = output_path(output_directory, source);
            if (process_image(source, destination) == 0) {
                processed++;
                printf("Serveur %ld: %s -> %s\n", (long)getpid(), source, destination);
            } else {
                fprintf(stderr, "Serveur %ld: échec pour %s\n", (long)getpid(), source);
            }
            fflush(stdout);
            free(destination);
        }
    }
    printf("Serveur image %ld: %d image(s) traitée(s), arrêt\n", (long)getpid(), processed);
    shm_detach(shared);
    return EXIT_SUCCESS;
}

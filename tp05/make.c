#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static char *object_name(const char *source)
{
    char *copy = xstrdup(source);
    char *base = basename(copy);
    char *dot = strrchr(base, '.');
    size_t length;
    char *object;

    if (dot != NULL && strcmp(dot, ".c") == 0) {
        *dot = '\0';
    }
    length = strlen(base) + 3;
    object = xmalloc(length);
    snprintf(object, length, "%s.o", base);
    free(copy);
    return object;
}

static pid_t compile(const char *source, const char *object)
{
    pid_t child = xfork();

    if (child == 0) {
        char *const arguments[] = {
            "gcc", "-Wall", "-Wextra", "-g", "-c", (char *)source, "-o", (char *)object, NULL
        };
        execvp(arguments[0], arguments);
        _exit(127);
    }
    return child;
}

int main(int argc, char **argv)
{
    const char *output;
    int source_count;
    char **objects;
    pid_t *children;
    int index;
    int failed = 0;

    if (argc < 3) {
        fprintf(stderr, "usage: %s source.c [source.c ...] executable\n", argv[0]);
        return EXIT_FAILURE;
    }
    source_count = argc - 2;
    output = argv[argc - 1];
    objects = xmalloc((size_t)source_count * sizeof(*objects));
    children = xmalloc((size_t)source_count * sizeof(*children));

    for (index = 0; index < source_count; index++) {
        objects[index] = object_name(argv[index + 1]);
        children[index] = compile(argv[index + 1], objects[index]);
        printf("Compilation: %s -> %s (pid %ld)\n",
               argv[index + 1], objects[index], (long)children[index]);
        fflush(stdout);
    }
    for (index = 0; index < source_count; index++) {
        int status;
        if (waitpid(children[index], &status, 0) < 0) {
            die("waitpid failed");
        }
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("Compilation réussie: %s\n", argv[index + 1]);
        } else {
            printf("Erreur de compilation: %s (code %d)\n", argv[index + 1],
                   WIFEXITED(status) ? WEXITSTATUS(status) : -1);
            failed = 1;
        }
    }
    if (failed) {
        fprintf(stderr, "édition de liens annulée\n");
        for (index = 0; index < source_count; index++) {
            free(objects[index]);
        }
        free(objects);
        free(children);
        return EXIT_FAILURE;
    }

    {
        pid_t child = xfork();
        if (child == 0) {
            char **arguments = xmalloc(((size_t)source_count + 4) * sizeof(*arguments));
            int position = 0;
            arguments[position++] = "gcc";
            arguments[position++] = "-o";
            arguments[position++] = (char *)output;
            for (int object = 0; object < source_count; object++) {
                arguments[position++] = objects[object];
            }
            arguments[position] = NULL;
            execvp(arguments[0], arguments);
            _exit(127);
        }
        {
            int status;
            if (waitpid(child, &status, 0) < 0) {
                die("waitpid failed");
            }
            if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
                fprintf(stderr, "édition de liens échouée\n");
                failed = 1;
            } else {
                printf("Exécutable créé: %s\n", output);
            }
        }
    }

    for (index = 0; index < source_count; index++) {
        free(objects[index]);
    }
    free(objects);
    free(children);
    return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}

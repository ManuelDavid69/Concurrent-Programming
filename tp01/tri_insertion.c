#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_array(const char *label, const int *values, size_t length)
{
    size_t index;

    printf("%s", label);
    for (index = 0; index < length; index++) {
        printf("%s%d", index == 0 ? "" : " ", values[index]);
    }
    printf("\n");
}

/*
 * Insertion sort.  At the beginning of iteration i, values[0..i-1] is sorted.
 * The key is moved to its place by shifting the larger elements one slot to
 * the right.
 */
static void insertion_sort(int *values, size_t length)
{
    size_t index;

    for (index = 1; index < length; index++) {
        int key = values[index];
        long position = (long)index - 1;

        while (position >= 0 && values[position] > key) {
            values[position + 1] = values[position];
            position--;
        }
        values[position + 1] = key;
    }
}

int main(int argc, char **argv)
{
    int default_values[] = {5, 2, 4, 6, 1, 3};
    int *values;
    size_t length;
    bool verbose = false;
    int index;

    if (argc >= 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0)) {
        verbose = true;
        argv++;
        argc--;
    }

    if (argc == 1) {
        length = sizeof(default_values) / sizeof(default_values[0]);
        values = xmalloc(length * sizeof(*values));
        for (index = 0; index < (int)length; index++) {
            values[index] = default_values[index];
        }
    } else {
        length = (size_t)(argc - 1);
        values = xmalloc(length * sizeof(*values));
        for (index = 1; index < argc; index++) {
            if (!parse_int(argv[index], &values[index - 1])) {
                fprintf(stderr, "%s: not an integer\n", argv[index]);
                free(values);
                return EXIT_FAILURE;
            }
        }
    }

    print_array("Avant: ", values, length);
    if (verbose) {
        size_t step;
        for (step = 1; step < length; step++) {
            int key = values[step];
            long position = (long)step - 1;
            while (position >= 0 && values[position] > key) {
                values[position + 1] = values[position];
                position--;
            }
            values[position + 1] = key;
            printf("Après insertion %zu: ", step);
            for (size_t i = 0; i < length; i++) {
                printf("%s%d", i == 0 ? "" : " ", values[i]);
            }
            printf("\n");
        }
    } else {
        insertion_sort(values, length);
    }
    print_array("Après: ", values, length);
    free(values);
    return EXIT_SUCCESS;
}

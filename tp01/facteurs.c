#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Smallest divisor greater than one.  Even numbers are handled first, then
 * only odd candidates are tested up to sqrt(n).  The loop condition uses
 * divisor <= n / divisor to avoid overflowing divisor * divisor.
 */
static long smallest_divisor(long number)
{
    long divisor;

    if (number % 2 == 0) {
        return 2;
    }
    for (divisor = 3; divisor <= number / divisor; divisor += 2) {
        if (number % divisor == 0) {
            return divisor;
        }
    }
    return number;
}

static void factorise(long number)
{
    long remaining = number;

    printf("%ld =", number);
    if (number < 0) {
        printf(" -1 x");
        remaining = -remaining;
    }
    if (remaining <= 1) {
        printf(" %ld\n", remaining);
        return;
    }
    while (remaining > 1) {
        long divisor = smallest_divisor(remaining);
        printf(" %ld", divisor);
        remaining /= divisor;
        if (remaining > 1) {
            printf(" x");
        }
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int index;

    if (argc < 2) {
        fprintf(stderr, "usage: %s number [number ...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    for (index = 1; index < argc; index++) {
        long number;

        if (!parse_long(argv[index], &number)) {
            fprintf(stderr, "%s: not an integer\n", argv[index]);
            return EXIT_FAILURE;
        }
        if (number == LONG_MIN) {
            fprintf(stderr, "%s: value is too small\n", argv[index]);
            return EXIT_FAILURE;
        }
        factorise(number);
    }
    return EXIT_SUCCESS;
}

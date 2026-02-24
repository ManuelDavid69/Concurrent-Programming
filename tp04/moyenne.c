#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    long sum = 0;
    int count = 0;
    int index;

    if (argc < 2) {
        printf("Aucune note fourni, moyenne impossible à calculer\n");
        return EXIT_FAILURE;
    }
    for (index = 1; index < argc; index++) {
        int grade;
        if (!parse_int(argv[index], &grade) || grade < 0 || grade > 20) {
            printf("Note(s) non valide(s)\n");
            return EXIT_FAILURE;
        }
        sum += grade;
        count++;
    }
    printf("Moyenne: %.2f\n", (double)sum / (double)count);
    return EXIT_SUCCESS;
}

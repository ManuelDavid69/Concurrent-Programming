#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "tas.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [options] [values...]\n"
            "  --random N [seed]   generate N random values\n"
            "  --sort              sort the values with heapsort\n"
            "  --display MODE      line, levels or tree (default: all)\n",
            program);
}

static bool parse_display(const char *text, TasAffichage *mode)
{
    if (strcmp(text, "line") == 0 || strcmp(text, "lineaire") == 0) {
        *mode = TAS_AFFICHAGE_LINEAIRE;
        return true;
    }
    if (strcmp(text, "levels") == 0 || strcmp(text, "niveaux") == 0) {
        *mode = TAS_AFFICHAGE_NIVEAUX;
        return true;
    }
    if (strcmp(text, "tree") == 0 || strcmp(text, "arbre") == 0) {
        *mode = TAS_AFFICHAGE_ARBRE;
        return true;
    }
    return false;
}

static void print_values(const char *label, const int *values, size_t length)
{
    size_t index;

    printf("%s", label);
    for (index = 0; index < length; index++) {
        printf("%s%d", index == 0 ? "" : " ", values[index]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    int default_values[] = {55, 38, 62, 40, 49, 42, 48, 51, 39};
    int *values = NULL;
    size_t count = 0;
    bool sort_mode = false;
    bool show_line = true;
    bool show_levels = true;
    bool show_tree = true;
    int index;

    for (index = 1; index < argc; index++) {
        if (strcmp(argv[index], "--random") == 0) {
            int amount;
            long seed;
            int generated;

            if (index + 1 >= argc || !parse_int(argv[index + 1], &amount) || amount < 0) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            index++;
            if (index + 1 < argc && parse_long(argv[index + 1], &seed)) {
                index++;
                srand((unsigned int)seed);
            } else {
                pc_seed();
            }
            values = xmalloc((size_t)amount * sizeof(*values));
            for (generated = 0; generated < amount; generated++) {
                values[generated] = rand_range(-1000, 1000);
            }
            count = (size_t)amount;
        } else if (strcmp(argv[index], "--sort") == 0) {
            sort_mode = true;
        } else if (strcmp(argv[index], "--display") == 0) {
            TasAffichage mode;
            if (index + 1 >= argc || !parse_display(argv[index + 1], &mode)) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            index++;
            show_line = mode == TAS_AFFICHAGE_LINEAIRE;
            show_levels = mode == TAS_AFFICHAGE_NIVEAUX;
            show_tree = mode == TAS_AFFICHAGE_ARBRE;
        } else if (strcmp(argv[index], "--help") == 0) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            int value;
            if (!parse_int(argv[index], &value)) {
                fprintf(stderr, "%s: not an integer\n", argv[index]);
                free(values);
                return EXIT_FAILURE;
            }
            values = xrealloc(values, (count + 1) * sizeof(*values));
            values[count++] = value;
        }
    }

    if (values == NULL || count == 0) {
        count = sizeof(default_values) / sizeof(default_values[0]);
        values = xmalloc(count * sizeof(*values));
        for (index = 0; index < (int)count; index++) {
            values[index] = default_values[index];
        }
    }

    if (sort_mode) {
        print_values("Avant: ", values, count);
        tri_par_tas(values, count);
        print_values("Après: ", values, count);
        for (index = 1; index < (int)count; index++) {
            if (values[index - 1] > values[index]) {
                fprintf(stderr, "internal error: heapsort failed\n");
                free(values);
                return EXIT_FAILURE;
            }
        }
        free(values);
        return EXIT_SUCCESS;
    }

    {
        Tas tas;
        size_t inserted;
        int extracted = 0;

        tas_init(&tas, count);
        for (inserted = 0; inserted < count; inserted++) {
            if (!tas_entasser(&tas, values[inserted])) {
                fprintf(stderr, "internal error: heap is full\n");
                tas_free(&tas);
                free(values);
                return EXIT_FAILURE;
            }
        }
        if (!tas_est_valide(&tas)) {
            fprintf(stderr, "internal error: invalid heap\n");
            tas_free(&tas);
            free(values);
            return EXIT_FAILURE;
        }
        if (show_line) {
            printf("Tas (linéaire): ");
            tas_afficher(&tas, TAS_AFFICHAGE_LINEAIRE);
        }
        if (show_levels) {
            printf("Tas (niveaux):\n");
            tas_afficher(&tas, TAS_AFFICHAGE_NIVEAUX);
        }
        if (show_tree) {
            printf("Tas (arbre):\n");
            tas_afficher(&tas, TAS_AFFICHAGE_ARBRE);
        }
        printf("Ordre d'extraction:");
        while (tas_detasser(&tas, &extracted)) {
            printf(" %d", extracted);
        }
        printf("\n");
        tas_free(&tas);
    }
    free(values);
    return EXIT_SUCCESS;
}

#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int *values;
    size_t length;
    size_t capacity;
} Sequence;

static void sequence_push(Sequence *sequence, int value)
{
    if (sequence->length == sequence->capacity) {
        sequence->capacity = sequence->capacity == 0 ? 16 : sequence->capacity * 2;
        sequence->values = xrealloc(sequence->values, sequence->capacity * sizeof(*sequence->values));
    }
    sequence->values[sequence->length++] = value;
}

static void sequence_free(Sequence *sequence)
{
    free(sequence->values);
    sequence->values = NULL;
    sequence->length = 0;
    sequence->capacity = 0;
}

static Sequence parse_initial(const char *text)
{
    Sequence sequence = {0};
    char *copy = xstrdup(text);
    char *token;
    char *save = NULL;

    for (token = strtok_r(copy, " ,", &save); token != NULL;
         token = strtok_r(NULL, " ,", &save)) {
        int value;
        if (!parse_int(token, &value) || value < 0) {
            fprintf(stderr, "invalid initial value: %s\n", token);
            free(copy);
            sequence_free(&sequence);
            exit(EXIT_FAILURE);
        }
        sequence_push(&sequence, value);
    }
    free(copy);
    if (sequence.length == 0) {
        sequence_push(&sequence, 1);
    }
    return sequence;
}

/*
 * Une étape de la suite « look-and-say » : chaque plage maximale de valeurs
 * égales est remplacée par (longueur de la plage, valeur).
 */
static Sequence next_line(const Sequence *current)
{
    Sequence next = {0};
    size_t index = 0;

    while (index < current->length) {
        int value = current->values[index];
        size_t run = 1;

        while (index + run < current->length && current->values[index + run] == value) {
            run++;
        }
        if (run > INT_MAX) {
            die("run is too long");
        }
        sequence_push(&next, (int)run);
        sequence_push(&next, value);
        index += run;
    }
    return next;
}

static void print_sequence(const Sequence *sequence)
{
    size_t index;

    for (index = 0; index < sequence->length; index++) {
        printf("%s%d", index == 0 ? "" : " ", sequence->values[index]);
    }
    printf("\n");
}

int main(int argc, char **argv)
{
    const char *initial_text = "3";
    int lines = 7;
    Sequence current;
    int line;

    if (argc >= 2) {
        initial_text = argv[1];
    }
    if (argc >= 3 && (!parse_int(argv[2], &lines) || lines <= 0)) {
        fprintf(stderr, "the number of lines must be a positive integer\n");
        return EXIT_FAILURE;
    }
    if (argc > 3) {
        fprintf(stderr, "usage: %s [initial values] [number of lines]\n", argv[0]);
        return EXIT_FAILURE;
    }

    current = parse_initial(initial_text);
    for (line = 0; line < lines; line++) {
        Sequence next;
        print_sequence(&current);
        if (line + 1 == lines) {
            break;
        }
        next = next_line(&current);
        sequence_free(&current);
        current = next;
    }
    sequence_free(&current);
    return EXIT_SUCCESS;
}

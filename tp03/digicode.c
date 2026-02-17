#define _POSIX_C_SOURCE 200809L

#include "automate.h"
#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Variant 3: the code ends with A or B.  Digits are accumulated in the
 * current attempt; A or B validates that attempt and clears it.  Typing more
 * digits than the code contains makes the attempt invalid, which is why
 * "78267B" is rejected and "7A8267B" is accepted.
 */
static int process_validation_code(const char *code)
{
    size_t code_length = strlen(code);
    size_t digit_count = code_length - 1;
    char attempt[AUTOMATE_MAX_CODE];
    size_t attempt_length = 0;
    int character;

    printf("Digicode %s: tapez les chiffres puis A ou B pour valider.\n", code);
    while ((character = getchar()) != EOF) {
        if (character == 'A' || character == 'B') {
            bool valid = attempt_length == digit_count
                && memcmp(attempt, code, digit_count) == 0
                && (char)character == code[digit_count];
            printf("%s\n", valid ? "Code valide" : "Code invalide");
            attempt_length = 0;
        } else if (character >= '0' && character <= '9') {
            if (attempt_length < digit_count) {
                attempt[attempt_length++] = (char)character;
            } else {
                /* Too many digits: the attempt can no longer match. */
                attempt_length = digit_count + 1;
            }
        }
    }
    return EXIT_SUCCESS;
}

static int process_dfa_code(const char *code, bool trace)
{
    Automate automate;
    int character;
    int opened = 0;

    if (!automate_init(&automate, code)) {
        fprintf(stderr, "invalid code: %s\n", code);
        return EXIT_FAILURE;
    }
    printf("Digicode %s: tapez les chiffres du code.\n", code);
    while ((character = getchar()) != EOF) {
        if (character < '0' || character > '9') {
            continue;
        }
        if (automate_feed(&automate, (char)character)) {
            printf("Porte ouverte\n");
            opened++;
        }
        if (trace) {
            printf("  '%c' -> état %d\n", (char)character, automate.state);
        }
    }
    if (opened == 0) {
        printf("Aucune séquence correcte\n");
    }
    return EXIT_SUCCESS;
}

static bool valid_dfa_code(const char *code)
{
    size_t index;

    if (*code == '\0' || strlen(code) >= AUTOMATE_MAX_CODE) {
        return false;
    }
    for (index = 0; code[index] != '\0'; index++) {
        if (code[index] < '0' || code[index] > '9') {
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv)
{
    const char *code = "5742";
    bool trace = false;
    int index;

    for (index = 1; index < argc; index++) {
        if (strcmp(argv[index], "--trace") == 0) {
            trace = true;
        } else if (strcmp(argv[index], "--help") == 0) {
            printf("usage: %s [code] [--trace]\n", argv[0]);
            printf("  default code: 5742\n");
            printf("  variant 2:    5551\n");
            printf("  variant 3:    8267B (validated with A or B)\n");
            return EXIT_SUCCESS;
        } else if (argv[index][0] == '-') {
            fprintf(stderr, "unknown option: %s\n", argv[index]);
            return EXIT_FAILURE;
        } else {
            code = argv[index];
        }
    }

    if (strlen(code) >= AUTOMATE_MAX_CODE) {
        fprintf(stderr, "code is too long\n");
        return EXIT_FAILURE;
    }
    if (code[strlen(code) - 1] == 'A' || code[strlen(code) - 1] == 'B') {
        return process_validation_code(code);
    }
    if (!valid_dfa_code(code)) {
        fprintf(stderr, "the code must contain digits only (or end with A/B)\n");
        return EXIT_FAILURE;
    }
    return process_dfa_code(code, trace);
}

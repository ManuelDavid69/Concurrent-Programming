#define _POSIX_C_SOURCE 200809L

#include "automate.h"

#include <stdio.h>
#include <string.h>

static void print_table(const char *code)
{
    Automate base;
    int state;
    int character;

    if (!automate_init(&base, code)) {
        return;
    }
    printf("Automate pour %s\n", code);
    printf("état |");
    for (character = '0'; character <= '9'; character++) {
        printf(" %c", character);
    }
    printf(" | sortie\n");
    printf("-----+----------------------+-------\n");
    for (state = 0; state < base.length; state++) {
        printf("  %d  |", state);
        for (character = '0'; character <= '9'; character++) {
            Automate copy = base;
            copy.state = state;
            int accepted = automate_feed(&copy, (char)character);
            printf(" %d", accepted ? 0 : copy.state);
        }
        printf(" | %s\n", state == base.length - 1 ? "ouvre" : "");
    }
    printf("\n");
}

static void print_validation_automaton(void)
{
    printf("Automate pour 8267B (validation par A ou B)\n");
    printf("état 0: attend 8        -> état 1\n");
    printf("état 1: attend 2        -> état 2\n");
    printf("état 2: attend 6        -> état 3\n");
    printf("état 3: attend 7        -> état 4\n");
    printf("état 4: attend B        -> code valide, retour à l'état 0\n");
    printf("A ou B sur un autre état, ou plus de 4 chiffres: code invalide\n");
    printf("Les chiffres sont accumulés depuis le dernier A/B, donc 7A8267B est valide\n");
    printf("mais 78267B ne l'est pas.\n\n");
}

int main(void)
{
    print_table("5742");
    print_table("5551");
    print_validation_automaton();
    return 0;
}

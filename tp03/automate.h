#ifndef TP03_AUTOMATE_H
#define TP03_AUTOMATE_H

#include <stdbool.h>
#include <stddef.h>

#define AUTOMATE_MAX_CODE 16

/*
 * Automate déterministe reconnaissant un motif avec la fonction d'échec de
 * KMP.  Lorsque le motif entier vient d'être lu, feed() renvoie true et
 * l'état revient à zéro, de sorte que les occurrences qui se chevauchent sont
 * trouvées.
 */
typedef struct {
    char code[AUTOMATE_MAX_CODE];
    int length;
    int prefix[AUTOMATE_MAX_CODE];
    int state;
} Automate;

bool automate_init(Automate *automate, const char *code);
int automate_feed(Automate *automate, char character);
void automate_reset(Automate *automate);

#endif

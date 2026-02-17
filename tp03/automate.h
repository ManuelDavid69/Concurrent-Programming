#ifndef TP03_AUTOMATE_H
#define TP03_AUTOMATE_H

#include <stdbool.h>
#include <stddef.h>

#define AUTOMATE_MAX_CODE 16

/*
 * Deterministic automaton recognising a pattern with the KMP failure
 * function.  When the whole pattern has just been read, feed() returns true
 * and the state goes back to zero, so overlapping occurrences are found.
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

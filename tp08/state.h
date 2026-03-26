#ifndef TP08_STATE_H
#define TP08_STATE_H

#include <sys/types.h>

#define TP08_SEM_KEY 0x08080801
#define TP08_SHM_KEY 0x08080802

enum {
    SEM_FIRST = 0,
    SEM_RDV2_BASE = 1,
    SEM_RDV3_BASE = 3,
    SEM_MUTEX = 6,
    SEM_EMETTEUR = 7,
    SEM_RECEPTEUR = 8,
    SEM_COUNT = 9
};

typedef struct {
    int emetteurs_attente;
    int recepteurs_attente;
    int messages;
} Etat;

#endif

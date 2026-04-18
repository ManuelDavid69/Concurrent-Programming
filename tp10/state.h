#ifndef TP10_STATE_H
#define TP10_STATE_H

#include <stddef.h>

#define TP10_RDV_SEM_KEY 0x0A0A0001
#define TP10_RDV_SHM_KEY 0x0A0A0002
#define TP10_PC_SEM_KEY 0x0A0A0003
#define TP10_PC_SHM_KEY 0x0A0A0004
#define TP10_EXCH_SEM_KEY 0x0A0A0005
#define TP10_EXCH_SHM_KEY 0x0A0A0006

enum {
    RDV_MUTEX = 0,
    RDV_TURNSTILE_0 = 1,
    RDV_TURNSTILE_1 = 2,
    RDV_SEM_COUNT = 3
};

enum {
    PC_MUTEX = 0,
    PC_EMPTY = 1,
    PC_FULL = 2,
    PC_SEM_COUNT = 3
};

enum {
    EXCH_P1 = 0,
    EXCH_P2 = 1,
    EXCH_P0_TO_P1 = 2,
    EXCH_P0_TO_P2 = 3,
    EXCH_SEM_COUNT = 4
};

typedef struct {
    int count;
    int generation;
} RendezVous;

typedef struct {
    int capacity;
    int head;
    int tail;
    int count;
    int producers_expected;
    int producers_started;
    int producers_active;
    int finished;
    int items[];
} Buffer;

typedef struct {
    int first;
    int second;
    int third;
} Echange;

#endif

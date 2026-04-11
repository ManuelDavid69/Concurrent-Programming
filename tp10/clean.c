#define _POSIX_C_SOURCE 200809L

#include "state.h"
#include "sysv.h"

#include <stdio.h>

int main(void)
{
    sem_remove_key(TP10_RDV_SEM_KEY);
    sem_remove_key(TP10_PC_SEM_KEY);
    sem_remove_key(TP10_EXCH_SEM_KEY);
    shm_remove_key(TP10_RDV_SHM_KEY);
    shm_remove_key(TP10_PC_SHM_KEY);
    shm_remove_key(TP10_EXCH_SHM_KEY);
    printf("Ressources du TP 10 supprimées.\n");
    return 0;
}

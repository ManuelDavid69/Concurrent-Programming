#define _POSIX_C_SOURCE 200809L

#include "state.h"
#include "sysv.h"

#include <stdio.h>

int main(void)
{
    sem_remove_key(TP11_CALC_SEM_KEY);
    sem_remove_key(TP11_IMG_SEM_KEY);
    shm_remove_key(TP11_CALC_SHM_KEY);
    shm_remove_key(TP11_IMG_SHM_KEY);
    printf("Ressources du TP 11 supprimées.\n");
    return 0;
}

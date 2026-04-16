#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "state.h"
#include "sysv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    unsigned short calc_values[CALC_SEM_COUNT] = {0};
    unsigned short image_values[IMG_SEM_COUNT] = {0};
    int calc_sem;
    int image_sem;
    int calc_shm;
    int image_shm;
    CalcShared *calculator;
    ImageShared *images;

    calc_values[CALC_MUTEX] = 1;
    calc_values[CALC_SLOTS] = CALC_QUEUE_CAPACITY;
    calc_values[CALC_ITEMS] = 0;
    for (int index = 0; index < CALC_MAX_CLIENTS; index++) {
        calc_values[CALC_RESULT_BASE + index] = 0;
    }
    image_values[IMG_MUTEX] = 1;
    image_values[IMG_SLOTS] = IMG_SLOTS;
    image_values[IMG_ITEMS] = 0;

    sem_remove_key(TP11_CALC_SEM_KEY);
    sem_remove_key(TP11_IMG_SEM_KEY);
    shm_remove_key(TP11_CALC_SHM_KEY);
    shm_remove_key(TP11_IMG_SHM_KEY);

    calc_sem = sem_create(TP11_CALC_SEM_KEY, CALC_SEM_COUNT, calc_values);
    image_sem = sem_create(TP11_IMG_SEM_KEY, IMG_SEM_COUNT, image_values);
    calc_shm = shm_create(TP11_CALC_SHM_KEY, sizeof(CalcShared));
    image_shm = shm_create(TP11_IMG_SHM_KEY, sizeof(ImageShared));
    if (calc_sem < 0 || image_sem < 0 || calc_shm < 0 || image_shm < 0) {
        sem_remove(calc_sem);
        sem_remove(image_sem);
        shm_remove(calc_shm);
        shm_remove(image_shm);
        die("cannot create the TP 11 resources");
    }

    calculator = shm_attach(calc_shm);
    images = shm_attach(image_shm);
    if (calculator == NULL || images == NULL) {
        die("cannot attach the shared segments");
    }
    memset(calculator, 0, sizeof(*calculator));
    memset(images, 0, sizeof(*images));
    shm_detach(calculator);
    shm_detach(images);
    printf("Ressources du TP 11 créées.\n");
    return EXIT_SUCCESS;
}

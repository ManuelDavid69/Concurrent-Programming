#define _POSIX_C_SOURCE 200809L

#include "tas.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void tas_init(Tas *tas, size_t capacity)
{
    tas->data = xmalloc((capacity == 0 ? 1 : capacity) * sizeof(*tas->data));
    tas->size = 0;
    tas->capacity = capacity;
}

void tas_free(Tas *tas)
{
    free(tas->data);
    tas->data = NULL;
    tas->size = 0;
    tas->capacity = 0;
}

static void swap(int *left, int *right)
{
    int temporary = *left;
    *left = *right;
    *right = temporary;
}

/* Moves the element at index up until its parent is smaller or equal. */
static void remonter(Tas *tas, size_t index)
{
    while (index > 0) {
        size_t parent = (index - 1) / 2;
        if (tas->data[parent] <= tas->data[index]) {
            break;
        }
        swap(&tas->data[parent], &tas->data[index]);
        index = parent;
    }
}

/* Moves the element at index down until both children are larger or equal. */
static void descendre(Tas *tas, size_t index)
{
    for (;;) {
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;
        size_t smallest = index;

        if (left < tas->size && tas->data[left] < tas->data[smallest]) {
            smallest = left;
        }
        if (right < tas->size && tas->data[right] < tas->data[smallest]) {
            smallest = right;
        }
        if (smallest == index) {
            return;
        }
        swap(&tas->data[index], &tas->data[smallest]);
        index = smallest;
    }
}

bool tas_entasser(Tas *tas, int value)
{
    if (tas->size == tas->capacity) {
        return false;
    }
    tas->data[tas->size] = value;
    remonter(tas, tas->size);
    tas->size++;
    return true;
}

bool tas_detasser(Tas *tas, int *value)
{
    if (tas->size == 0) {
        return false;
    }
    *value = tas->data[0];
    tas->size--;
    if (tas->size > 0) {
        tas->data[0] = tas->data[tas->size];
        descendre(tas, 0);
    }
    return true;
}

bool tas_racine(const Tas *tas, int *value)
{
    if (tas->size == 0) {
        return false;
    }
    *value = tas->data[0];
    return true;
}

bool tas_est_valide(const Tas *tas)
{
    size_t index;

    for (index = 1; index < tas->size; index++) {
        size_t parent = (index - 1) / 2;
        if (tas->data[parent] > tas->data[index]) {
            return false;
        }
    }
    return true;
}

static void afficher_lineaire(const Tas *tas)
{
    size_t index;

    for (index = 0; index < tas->size; index++) {
        printf("%s%d", index == 0 ? "" : " ", tas->data[index]);
    }
    printf("\n");
}

static void afficher_niveaux(const Tas *tas)
{
    size_t start = 0;
    size_t level_size = 1;

    while (start < tas->size) {
        size_t end = start + level_size;
        size_t index;

        if (end > tas->size) {
            end = tas->size;
        }
        for (index = start; index < end; index++) {
            printf("%s%d", index == start ? "" : " ", tas->data[index]);
        }
        printf("\n");
        start = end;
        level_size *= 2;
    }
}

static void afficher_arbre_rec(const Tas *tas, size_t index, const char *prefix, bool last)
{
    char *child_prefix;
    size_t left;
    size_t right;
    size_t length = strlen(prefix);

    printf("%s%s%d\n", prefix, last ? "\\-- " : "|-- ", tas->data[index]);

    child_prefix = xmalloc(length + 5);
    memcpy(child_prefix, prefix, length);
    memcpy(child_prefix + length, last ? "    " : "|   ", 5);

    left = 2 * index + 1;
    right = 2 * index + 2;
    if (left < tas->size) {
        afficher_arbre_rec(tas, left, child_prefix, right >= tas->size);
    }
    if (right < tas->size) {
        afficher_arbre_rec(tas, right, child_prefix, true);
    }
    free(child_prefix);
}

static void afficher_arbre(const Tas *tas)
{
    if (tas->size == 0) {
        printf("(tas vide)\n");
        return;
    }
    printf("%d\n", tas->data[0]);
    if (tas->size > 1) {
        afficher_arbre_rec(tas, 1, "", 2 >= tas->size);
    }
    if (tas->size > 2) {
        afficher_arbre_rec(tas, 2, "", true);
    }
}

void tas_afficher(const Tas *tas, TasAffichage mode)
{
    switch (mode) {
    case TAS_AFFICHAGE_LINEAIRE:
        afficher_lineaire(tas);
        break;
    case TAS_AFFICHAGE_NIVEAUX:
        afficher_niveaux(tas);
        break;
    case TAS_AFFICHAGE_ARBRE:
        afficher_arbre(tas);
        break;
    }
}

static void descendre_max(int *values, size_t size, size_t index)
{
    for (;;) {
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;
        size_t largest = index;

        if (left < size && values[left] > values[largest]) {
            largest = left;
        }
        if (right < size && values[right] > values[largest]) {
            largest = right;
        }
        if (largest == index) {
            return;
        }
        swap(&values[index], &values[largest]);
        index = largest;
    }
}

void tri_par_tas(int *values, size_t length)
{
    size_t index;

    if (length < 2) {
        return;
    }
    /* Build a max-heap in place. */
    for (index = length / 2; index > 0; index--) {
        descendre_max(values, length, index - 1);
    }
    /* Move the maximum to the end, then restore the heap on the prefix. */
    for (index = length; index > 1; index--) {
        swap(&values[0], &values[index - 1]);
        descendre_max(values, index - 1, 0);
    }
}

#ifndef TP02_TAS_H
#define TP02_TAS_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Min-heap stored in an array.  For a node at index i:
 *   left child  = 2 * i + 1
 *   right child = 2 * i + 2
 *   parent      = (i - 1) / 2
 */
typedef struct {
    int *data;
    size_t size;
    size_t capacity;
} Tas;

typedef enum {
    TAS_AFFICHAGE_LINEAIRE,
    TAS_AFFICHAGE_NIVEAUX,
    TAS_AFFICHAGE_ARBRE
} TasAffichage;

void tas_init(Tas *tas, size_t capacity);
void tas_free(Tas *tas);
bool tas_entasser(Tas *tas, int value);
bool tas_detasser(Tas *tas, int *value);
bool tas_racine(const Tas *tas, int *value);
bool tas_est_valide(const Tas *tas);
void tas_afficher(const Tas *tas, TasAffichage mode);

/* In-place heap sort (ascending order) using a max-heap. */
void tri_par_tas(int *values, size_t length);

#endif

#ifndef PC_COMMON_H
#define PC_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

/* ------------------------------------------------------------------------- */
/* Erreurs et allocation                                                     */
/* ------------------------------------------------------------------------- */

void die(const char *format, ...)
    __attribute__((noreturn, format(printf, 1, 2)));

void *xmalloc(size_t size);
void *xcalloc(size_t count, size_t size);
void *xrealloc(void *pointer, size_t size);
char *xstrdup(const char *string);

/* ------------------------------------------------------------------------- */
/* Analyse syntaxique et nombres aléatoires                                  */
/* ------------------------------------------------------------------------- */

/* Analyse stricte d'un entier : toute la chaîne doit être un int valide. */
bool parse_int(const char *text, int *value);
bool parse_long(const char *text, long *value);
bool parse_double(const char *text, double *value);

/* Initialise rand() à partir de l'horloge et du pid. */
void pc_seed(void);
int rand_range(int minimum, int maximum);

/* ------------------------------------------------------------------------- */
/* Temporisation                                                             */
/* ------------------------------------------------------------------------- */

/*
 * Tous les TP qui simulent un travail utilisent ces fonctions auxiliaires.
 * PC_SLEEP_SCALE (un nombre à virgule flottante positif, 1.0 par défaut)
 * multiplie chaque durée, ainsi une valeur inférieure à 1 accélère les tests
 * d'intégration sans modifier les programmes.
 */
double pc_sleep_scale(void);
void pc_sleep(double seconds);
int pc_delay_ms(int milliseconds);

/* ------------------------------------------------------------------------- */
/* Descripteurs de fichiers                                                  */
/* ------------------------------------------------------------------------- */

/* Écrit exactement count octets, en réessayant sur EINTR et les écritures partielles. */
int write_all(int fd, const void *buffer, size_t count);

/* Lit exactement count octets, en réessayant sur EINTR et les lectures partielles. */
int read_all(int fd, void *buffer, size_t count);

/* Ferme un descripteur et le remet à -1. */
void close_fd(int *fd);

/* ------------------------------------------------------------------------- */
/* Processus                                                                 */
/* ------------------------------------------------------------------------- */

pid_t xfork(void);

#endif

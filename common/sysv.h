#ifndef PC_SYSV_H
#define PC_SYSV_H

#include <stddef.h>
#include <sys/ipc.h>
#include <sys/types.h>

/* Crée un ensemble de sémaphores et initialise chaque sémaphore. */
int sem_create(key_t key, int count, const unsigned short *values);

/* Ouvre un ensemble existant. */
int sem_open(key_t key, int count);

/* Supprime un ensemble par clé ou par identifiant ; les ressources absentes sont ignorées. */
void sem_remove_key(key_t key);
void sem_remove(int identifier);

/* Opérations P et V classiques.  sem_p réessaie sur EINTR. */
int sem_p(int identifier, int index);
int sem_v(int identifier, int index);

/* P qui renvoie -1 avec errno == EINTR au lieu de réessayer. */
int sem_p_interruptible(int identifier, int index);

/* P non bloquant : renvoie 0 en cas de succès, -1 avec errno EAGAIN si indisponible. */
int sem_try_p(int identifier, int index);

/* Crée ou ouvre un segment de mémoire partagée et l'attache. */
int shm_create(key_t key, size_t size);
int shm_open(key_t key, size_t size);
void shm_remove_key(key_t key);
void shm_remove(int identifier);
void *shm_attach(int identifier);
void shm_detach(void *address);

#endif

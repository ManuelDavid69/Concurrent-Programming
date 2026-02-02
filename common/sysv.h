#ifndef PC_SYSV_H
#define PC_SYSV_H

#include <stddef.h>
#include <sys/ipc.h>
#include <sys/types.h>

/* Creates a semaphore set and initialises every semaphore. */
int sem_create(key_t key, int count, const unsigned short *values);

/* Opens an existing set. */
int sem_open(key_t key, int count);

/* Removes a set by key or by identifier; missing resources are ignored. */
void sem_remove_key(key_t key);
void sem_remove(int identifier);

/* Classic P and V operations.  sem_p retries on EINTR. */
int sem_p(int identifier, int index);
int sem_v(int identifier, int index);

/* P that returns -1 with errno == EINTR instead of retrying. */
int sem_p_interruptible(int identifier, int index);

/* Non-blocking P: returns 0 on success, -1 with errno EAGAIN if unavailable. */
int sem_try_p(int identifier, int index);

/* Creates or opens a shared-memory segment and attaches it. */
int shm_create(key_t key, size_t size);
int shm_open(key_t key, size_t size);
void shm_remove_key(key_t key);
void shm_remove(int identifier);
void *shm_attach(int identifier);
void shm_detach(void *address);

#endif

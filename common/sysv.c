#define _POSIX_C_SOURCE 200809L

#include "sysv.h"
#include "common.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>

#if defined(__linux__)
union semun {
    int value;
    struct semid_ds *buffer;
    unsigned short *array;
};
#endif

static int semaphore_operation(int identifier, int index, int delta, int flags)
{
    struct sembuf operation;

    operation.sem_num = (unsigned short)index;
    operation.sem_op = (short)delta;
    operation.sem_flg = (short)flags;
    return semop(identifier, &operation, 1);
}

int sem_create(key_t key, int count, const unsigned short *values)
{
    union semun argument;
    int identifier;

    if (count <= 0 || values == NULL) {
        errno = EINVAL;
        return -1;
    }
    identifier = semget(key, count, IPC_CREAT | IPC_EXCL | 0666);
    if (identifier < 0) {
        return -1;
    }
    argument.array = (unsigned short *)values;
    if (semctl(identifier, 0, SETALL, argument) < 0) {
        int saved = errno;
        semctl(identifier, 0, IPC_RMID);
        errno = saved;
        return -1;
    }
    return identifier;
}

int sem_open(key_t key, int count)
{
    return semget(key, count, 0666);
}

void sem_remove_key(key_t key)
{
    int identifier = semget(key, 0, 0666);
    if (identifier >= 0) {
        semctl(identifier, 0, IPC_RMID);
    }
}

void sem_remove(int identifier)
{
    if (identifier >= 0) {
        semctl(identifier, 0, IPC_RMID);
    }
}

int sem_p(int identifier, int index)
{
    for (;;) {
        if (semaphore_operation(identifier, index, -1, SEM_UNDO) == 0) {
            return 0;
        }
        if (errno != EINTR) {
            return -1;
        }
    }
}

int sem_v(int identifier, int index)
{
    return semaphore_operation(identifier, index, 1, SEM_UNDO);
}

int sem_p_interruptible(int identifier, int index)
{
    return semaphore_operation(identifier, index, -1, SEM_UNDO);
}

int sem_try_p(int identifier, int index)
{
    return semaphore_operation(identifier, index, -1, SEM_UNDO | IPC_NOWAIT);
}

int shm_create(key_t key, size_t size)
{
    return shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);
}

int shm_open(key_t key, size_t size)
{
    return shmget(key, size, 0666);
}

void shm_remove_key(key_t key)
{
    int identifier = shmget(key, 0, 0666);
    if (identifier >= 0) {
        shmctl(identifier, IPC_RMID, NULL);
    }
}

void shm_remove(int identifier)
{
    if (identifier >= 0) {
        shmctl(identifier, IPC_RMID, NULL);
    }
}

void *shm_attach(int identifier)
{
    void *address = shmat(identifier, NULL, 0);
    if (address == (void *)-1) {
        return NULL;
    }
    return address;
}

void shm_detach(void *address)
{
    if (address != NULL) {
        shmdt(address);
    }
}

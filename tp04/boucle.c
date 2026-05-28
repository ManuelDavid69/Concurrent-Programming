#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * fork() est appelé trois fois dans une boucle, sans aucune condition sur
 * l'appel ni sur l'affichage.  Chaque processus poursuit donc la boucle, ce
 * qui produit l'arbre binaire complet des processus (2^3 processus).
 */
int main(void)
{
    int iteration;

    for (iteration = 0; iteration < 3; iteration++) {
        pid_t result = xfork();
        printf("%d: Je suis le processus %ld, mon père est le processus %ld, fork a retourné %ld\n",
               iteration, (long)getpid(), (long)getppid(), (long)result);
        fflush(stdout);
    }
    while (wait(NULL) > 0) {
        /* Récupère chaque descendant pour que l'invite du shell revienne proprement. */
    }
    return EXIT_SUCCESS;
}

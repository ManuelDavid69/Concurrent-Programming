#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Version modifiée du cas 2.  Chaque père attend son fils avant d'afficher,
 * ainsi la feuille (E) s'affiche en premier et la racine (A) en dernier :
 * E D C B A.
 */
int main(void)
{
    int i = 0;

    for (i = 0; i < 4; i++) {
        pid_t child = fork();
        if (child < 0) {
            perror("fork");
            return 1;
        }
        if (child > 0) {
            waitpid(child, NULL, 0);
            printf("Je suis %c\n", 'A' + i);
            return 0;
        }
    }
    printf("Je suis %c\n", 'A' + i);
    return 0;
}

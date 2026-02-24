#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

/*
 * Modified version of case 2.  Each parent waits for its child before
 * printing, so the leaf (E) prints first and the root (A) last: E D C B A.
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

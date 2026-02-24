#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define PROCESS 4
#define TAB_SIZE 100

int main(void)
{
    int i = 0;
    int pere = 1;
    int tab[100] = {0};

    for (i = 0; i < TAB_SIZE; i++) {
        tab[i] = i % 10;
    }
    for (i = 0; i < PROCESS; i++) {
        if (fork() == 0) {
            pere = 0;
            break;
        }
    }
    if (pere) {
        for (i = 0; i < PROCESS; i++) {
            wait(NULL);
        }
        puts("Calculs terminés");
    } else {
        int somme = 0;
        for (int j = i * 25; j < (i + 1) * 25; j++) {
            somme += tab[j];
        }
        printf("%d %d\n", i, somme);
    }
    return 0;
}

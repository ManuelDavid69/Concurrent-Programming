#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
    int i = 0;

    for (i = 0; i < 4; i++) {
        if (fork()) {
            break;
        }
    }
    srand((unsigned int)getpid());
    {
        int delai = rand() % 4;
        pc_sleep(delai);
        printf("Je suis %c, j'ai dormi %d s\n", 'A' + i, delai);
    }
    return 0;
}

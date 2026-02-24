#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    int i;
    int n = 0;
    pid_t fils_pid;

    for (i = 1; i < 5; i++) {
        fils_pid = fork();
        if (fils_pid > 0) {
            n = i * 2;
            break;
        }
    }
    printf("%d\n", n);
    while (wait(NULL) > 0) {
        /* reap the descendants created by the children */
    }
    return 0;
}

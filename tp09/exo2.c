#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
        die("signal failed");
    }
    for (;;) {
        printf("SIGINT est ignoré, utilisez SIGKILL pour arrêter ce programme\n");
        fflush(stdout);
        pc_sleep(1);
    }
}

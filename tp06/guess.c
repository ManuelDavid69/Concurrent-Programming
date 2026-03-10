#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int choose_secret(void)
{
    const char *text = getenv("GUESS_SECRET");
    int secret;

    if (text != NULL && parse_int(text, &secret) && secret > 0 && secret <= 1000) {
        return secret;
    }
    pc_seed();
    return rand_range(1, 1000);
}

static void child_loop(int guesses_read, int replies_write, int secret)
{
    int guess;
    char reply;

    while (read_all(guesses_read, &guess, sizeof(guess)) == 0) {
        if (guess == secret) {
            reply = '=';
        } else if (guess < secret) {
            reply = '+';
        } else {
            reply = '-';
        }
        if (write_all(replies_write, &reply, 1) != 0) {
            break;
        }
        if (reply == '=') {
            break;
        }
    }
    close(guesses_read);
    close(replies_write);
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    int to_child[2];
    int to_parent[2];
    int secret = choose_secret();
    pid_t child;
    char line[128];
    int finished = 0;

    if (pipe(to_child) < 0 || pipe(to_parent) < 0) {
        die("pipe failed");
    }
    child = xfork();
    if (child == 0) {
        close(to_child[1]);
        close(to_parent[0]);
        child_loop(to_child[0], to_parent[1], secret);
    }
    close(to_child[0]);
    close(to_parent[1]);

    printf("Devinez le nombre entre 1 et 1000 (le fils a choisi un secret).\n");
    while (!finished) {
        int guess;
        char reply;

        printf("Votre proposition: ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }
        line[strcspn(line, "\r\n")] = '\0';
        if (!parse_int(line, &guess) || guess <= 0 || guess > 1000) {
            printf("Entrez un entier entre 1 et 1000.\n");
            continue;
        }
        if (write_all(to_child[1], &guess, sizeof(guess)) != 0
            || read_all(to_parent[0], &reply, 1) != 0) {
            fprintf(stderr, "communication with the child failed\n");
            break;
        }
        switch (reply) {
        case '+':
            printf("Trop petit\n");
            break;
        case '-':
            printf("Trop grand\n");
            break;
        case '=':
            printf("Trouvé !\n");
            finished = 1;
            break;
        default:
            fprintf(stderr, "invalid reply from the child\n");
            finished = 1;
            break;
        }
    }
    close(to_child[1]);
    close(to_parent[0]);
    waitpid(child, NULL, 0);
    return finished ? EXIT_SUCCESS : EXIT_FAILURE;
}

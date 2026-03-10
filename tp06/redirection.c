#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int wait_child(pid_t child, const char *label)
{
    int status;

    if (waitpid(child, &status, 0) < 0) {
        die("waitpid failed");
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        fprintf(stderr, "%s failed\n", label);
        return -1;
    }
    return 0;
}

static int simple_redirection(void)
{
    pid_t child = xfork();

    if (child == 0) {
        int input = open("/etc/passwd", O_RDONLY);
        if (input < 0) {
            die("cannot open /etc/passwd");
        }
        if (dup2(input, STDIN_FILENO) < 0) {
            die("dup2 failed");
        }
        close(input);
        execlp("wc", "wc", (char *)NULL);
        die("execvp wc failed");
    }
    return wait_child(child, "wc") == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int simple_pipe(void)
{
    int channel[2];
    pid_t cut;
    pid_t tr;
    int result = 0;

    if (pipe(channel) < 0) {
        die("pipe failed");
    }
    cut = xfork();
    if (cut == 0) {
        close(channel[0]);
        if (dup2(channel[1], STDOUT_FILENO) < 0) {
            die("dup2 failed");
        }
        close(channel[1]);
        execlp("cut", "cut", "-f", "1", "-d", ":", "/etc/passwd", (char *)NULL);
        die("execvp cut failed");
    }
    tr = xfork();
    if (tr == 0) {
        close(channel[1]);
        if (dup2(channel[0], STDIN_FILENO) < 0) {
            die("dup2 failed");
        }
        close(channel[0]);
        execlp("tr", "tr", "a-z", "A-Z", (char *)NULL);
        die("execvp tr failed");
    }
    close(channel[0]);
    close(channel[1]);
    if (wait_child(cut, "cut") != 0 || wait_child(tr, "tr") != 0) {
        result = 1;
    }
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int full_pipeline(void)
{
    int first_pipe[2];
    int second_pipe[2];
    int input;
    int output;
    pid_t cut;
    pid_t sed;
    pid_t sort;
    int result = 0;

    input = open("/etc/passwd", O_RDONLY);
    output = open("users", O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (input < 0 || output < 0 || pipe(first_pipe) < 0 || pipe(second_pipe) < 0) {
        die("cannot prepare the pipeline");
    }

    cut = xfork();
    if (cut == 0) {
        close(first_pipe[0]);
        close(second_pipe[0]);
        close(second_pipe[1]);
        dup2(input, STDIN_FILENO);
        dup2(first_pipe[1], STDOUT_FILENO);
        close(input);
        close(output);
        close(first_pipe[1]);
        execlp("cut", "cut", "-f", "1,3", "-d", ":", (char *)NULL);
        die("execvp cut failed");
    }

    sed = xfork();
    if (sed == 0) {
        close(first_pipe[1]);
        close(second_pipe[0]);
        dup2(first_pipe[0], STDIN_FILENO);
        dup2(second_pipe[1], STDOUT_FILENO);
        close(input);
        close(output);
        close(first_pipe[0]);
        close(second_pipe[1]);
        execlp("sed", "sed", "s+^\\(.*\\):\\(.*\\)+\\2:\\1+", (char *)NULL);
        die("execvp sed failed");
    }

    sort = xfork();
    if (sort == 0) {
        close(first_pipe[0]);
        close(first_pipe[1]);
        close(second_pipe[1]);
        dup2(second_pipe[0], STDIN_FILENO);
        dup2(output, STDOUT_FILENO);
        close(input);
        close(output);
        close(second_pipe[0]);
        execlp("sort", "sort", "-n", (char *)NULL);
        die("execvp sort failed");
    }

    close(input);
    close(output);
    close(first_pipe[0]);
    close(first_pipe[1]);
    close(second_pipe[0]);
    close(second_pipe[1]);
    if (wait_child(cut, "cut") != 0 || wait_child(sed, "sed") != 0
        || wait_child(sort, "sort") != 0) {
        result = 1;
    }
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s simple|pipe|pipeline\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "simple") == 0) {
        return simple_redirection();
    }
    if (strcmp(argv[1], "pipe") == 0) {
        return simple_pipe();
    }
    if (strcmp(argv[1], "pipeline") == 0) {
        return full_pipeline();
    }
    fprintf(stderr, "unknown mode: %s\n", argv[1]);
    return EXIT_FAILURE;
}

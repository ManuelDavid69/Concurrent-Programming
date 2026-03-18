#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    int client_id;
    int left;
    int right;
    char operator;
} Request;

typedef struct {
    int left;
    int right;
    int result;
    char operator;
} Response;

static int evaluate(const Request *request, Response *response)
{
    response->left = request->left;
    response->right = request->right;
    response->operator = request->operator;
    switch (request->operator) {
    case '+':
        response->result = request->left + request->right;
        return 0;
    case '-':
        response->result = request->left - request->right;
        return 0;
    case '*':
        response->result = request->left * request->right;
        return 0;
    case '/':
        if (request->right == 0) {
            return -1;
        }
        response->result = request->left / request->right;
        return 0;
    default:
        return -1;
    }
}

static void client_process(int id, int request_write, int response_read, unsigned int seed)
{
    static const char operators[] = {'+', '-', '*', '/'};
    Request request;
    Response response;

    srand(seed);
    request.client_id = id;
    request.left = rand_range(0, 99);
    request.right = rand_range(1, 99);
    request.operator = operators[rand_range(0, 3)];
    if (write_all(request_write, &request, sizeof(request)) != 0) {
        die("client %d cannot send its request", id);
    }
    close(request_write);
    if (read_all(response_read, &response, sizeof(response)) != 0) {
        die("client %d cannot read its response", id);
    }
    close(response_read);
    printf("Client %d: %d %c %d = %d\n", id, response.left, response.operator,
           response.right, response.result);
    fflush(stdout);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    int client_count;
    int request_pipe[2];
    int (*response_pipes)[2];
    pid_t *clients;
    unsigned int base_seed;
    const char *seed_text;
    int index;

    if (argc != 2 || !parse_int(argv[1], &client_count) || client_count < 1
        || client_count > 32) {
        fprintf(stderr, "usage: %s N (1 <= N <= 32)\n", argv[0]);
        return EXIT_FAILURE;
    }
    seed_text = getenv("CALC_SEED");
    if (seed_text != NULL) {
        int parsed;
        if (!parse_int(seed_text, &parsed)) {
            die("invalid CALC_SEED");
        }
        base_seed = (unsigned int)parsed;
    } else {
        base_seed = (unsigned int)(time(NULL) ^ getpid());
    }

    response_pipes = xmalloc((size_t)client_count * sizeof(*response_pipes));
    clients = xmalloc((size_t)client_count * sizeof(*clients));
    if (pipe(request_pipe) < 0) {
        die("pipe failed");
    }
    for (index = 0; index < client_count; index++) {
        if (pipe(response_pipes[index]) < 0) {
            die("pipe failed");
        }
    }

    for (index = 0; index < client_count; index++) {
        pid_t child = xfork();
        if (child == 0) {
            int other;
            close(request_pipe[0]);
            for (other = 0; other < client_count; other++) {
                if (other != index) {
                    close(response_pipes[other][0]);
                    close(response_pipes[other][1]);
                }
            }
            close(response_pipes[index][1]);
            client_process(index, request_pipe[1], response_pipes[index][0],
                           base_seed + (unsigned int)index * 7919U);
        }
        clients[index] = child;
    }

    close(request_pipe[1]);
    for (index = 0; index < client_count; index++) {
        close(response_pipes[index][0]);
    }
    for (index = 0; index < client_count; index++) {
        Request request;
        Response response;

        if (read_all(request_pipe[0], &request, sizeof(request)) != 0) {
            die("the server cannot read a request");
        }
        if (request.client_id < 0 || request.client_id >= client_count) {
            die("invalid client id %d", request.client_id);
        }
        if (evaluate(&request, &response) != 0) {
            response.left = request.left;
            response.right = request.right;
            response.operator = request.operator;
            response.result = 0;
            fprintf(stderr, "invalid request from client %d\n", request.client_id);
        }
        if (write_all(response_pipes[request.client_id][1], &response, sizeof(response)) != 0) {
            die("the server cannot send a response");
        }
    }
    close(request_pipe[0]);
    for (index = 0; index < client_count; index++) {
        close(response_pipes[index][1]);
    }
    for (index = 0; index < client_count; index++) {
        int status;
        if (waitpid(clients[index], &status, 0) < 0) {
            die("waitpid failed");
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            die("client %d failed", index);
        }
    }
    free(response_pipes);
    free(clients);
    return EXIT_SUCCESS;
}

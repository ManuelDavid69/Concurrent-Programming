#define _POSIX_C_SOURCE 200809L

#include "common.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void die(const char *format, ...)
{
    va_list arguments;

    fprintf(stderr, "error: ");
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

void *xmalloc(size_t size)
{
    void *pointer = malloc(size == 0 ? 1 : size);
    if (pointer == NULL) {
        die("out of memory (%zu bytes)", size);
    }
    return pointer;
}

void *xcalloc(size_t count, size_t size)
{
    void *pointer = calloc(count == 0 ? 1 : count, size == 0 ? 1 : size);
    if (pointer == NULL) {
        die("out of memory (%zu x %zu bytes)", count, size);
    }
    return pointer;
}

void *xrealloc(void *pointer, size_t size)
{
    void *result = realloc(pointer, size == 0 ? 1 : size);
    if (result == NULL) {
        die("out of memory (%zu bytes)", size);
    }
    return result;
}

char *xstrdup(const char *string)
{
    size_t length = strlen(string) + 1;
    char *copy = xmalloc(length);
    memcpy(copy, string, length);
    return copy;
}

bool parse_int(const char *text, int *value)
{
    char *end = NULL;
    long parsed;

    if (text == NULL || *text == '\0') {
        return false;
    }
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed < INT_MIN
        || parsed > INT_MAX) {
        return false;
    }
    *value = (int)parsed;
    return true;
}

bool parse_long(const char *text, long *value)
{
    char *end = NULL;
    long parsed;

    if (text == NULL || *text == '\0') {
        return false;
    }
    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') {
        return false;
    }
    *value = parsed;
    return true;
}

bool parse_double(const char *text, double *value)
{
    char *end = NULL;
    double parsed;

    if (text == NULL || *text == '\0') {
        return false;
    }
    errno = 0;
    parsed = strtod(text, &end);
    if (errno != 0 || end == text || *end != '\0' || !isfinite(parsed)) {
        return false;
    }
    *value = parsed;
    return true;
}

void pc_seed(void)
{
    unsigned int seed = (unsigned int)time(NULL);
    seed ^= (unsigned int)getpid() << 16;
    seed ^= (unsigned int)clock();
    srand(seed);
}

int rand_range(int minimum, int maximum)
{
    int range;

    if (maximum <= minimum) {
        return minimum;
    }
    range = maximum - minimum + 1;
    return minimum + rand() % range;
}

double pc_sleep_scale(void)
{
    const char *text = getenv("PC_SLEEP_SCALE");
    char *end = NULL;
    double scale;

    if (text == NULL || *text == '\0') {
        return 1.0;
    }
    scale = strtod(text, &end);
    if (end == text || *end != '\0' || !isfinite(scale) || scale <= 0.0) {
        return 1.0;
    }
    if (scale > 1000.0) {
        scale = 1000.0;
    }
    return scale;
}

void pc_sleep(double seconds)
{
    struct timespec request;
    struct timespec remaining;
    double scaled = seconds / pc_sleep_scale();

    if (scaled <= 0.0) {
        return;
    }
    request.tv_sec = (time_t)scaled;
    request.tv_nsec = (long)((scaled - (double)request.tv_sec) * 1e9);
    while (nanosleep(&request, &remaining) == -1 && errno == EINTR) {
        request = remaining;
    }
}

int pc_delay_ms(int milliseconds)
{
    double scaled = (double)milliseconds / pc_sleep_scale();
    int result = (int)(scaled + 0.5);

    return result < 1 ? 1 : result;
}

int write_all(int fd, const void *buffer, size_t count)
{
    const unsigned char *bytes = buffer;

    while (count > 0) {
        ssize_t written = write(fd, bytes, count);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        bytes += (size_t)written;
        count -= (size_t)written;
    }
    return 0;
}

int read_all(int fd, void *buffer, size_t count)
{
    unsigned char *bytes = buffer;

    while (count > 0) {
        ssize_t received = read(fd, bytes, count);
        if (received < 0) {
            if (errno == EINTR) {
                continue;
            }
            return -1;
        }
        if (received == 0) {
            return -1;
        }
        bytes += (size_t)received;
        count -= (size_t)received;
    }
    return 0;
}

void close_fd(int *fd)
{
    if (*fd >= 0) {
        close(*fd);
        *fd = -1;
    }
}

pid_t xfork(void)
{
    pid_t child = fork();

    if (child < 0) {
        die("fork failed: %s", strerror(errno));
    }
    return child;
}

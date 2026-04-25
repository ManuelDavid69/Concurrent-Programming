#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "image.h"

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ITERATIONS 500

static int canvas_width = 800;
static int canvas_height = 600;

typedef struct {
    Image *image;
    int thread_index;
    int thread_count;
} ThreadArgument;

/*
 * The calculation function from the statement: compute the colour of the
 * Mandelbrot pixel (x, y) and store the RGB triplet at *pixel.
 */
static void calcul(int x, int y, unsigned char *pixel)
{
    double x0 = (x - canvas_width / 2.0) * 4.0 / canvas_width;
    double y0 = (y - canvas_height / 2.0) * 4.0 / canvas_height;
    double real = 0.0;
    double imaginary = 0.0;
    int iteration = 0;

    while (real * real + imaginary * imaginary <= 4.0 && iteration < MAX_ITERATIONS) {
        double next_real = real * real - imaginary * imaginary + x0;
        imaginary = 2.0 * real * imaginary + y0;
        real = next_real;
        iteration++;
    }
    if (iteration == MAX_ITERATIONS) {
        pixel[0] = 0;
        pixel[1] = 0;
        pixel[2] = 0;
    } else {
        double normalized = (double)iteration / MAX_ITERATIONS;
        pixel[0] = (unsigned char)(255.0 * pow(normalized, 0.4));
        pixel[1] = (unsigned char)(255.0 * pow(normalized, 0.7));
        pixel[2] = (unsigned char)(255.0 * pow(normalized, 1.0));
    }
}

static void *worker(void *argument)
{
    ThreadArgument *thread = argument;

    for (int y = thread->thread_index; y < thread->image->height; y += thread->thread_count) {
        for (int x = 0; x < thread->image->width; x++) {
            calcul(x, y, image_pixel(thread->image, x, y));
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    int thread_count = 4;
    int width = 800;
    int height = 600;
    const char *output = "fractale.ppm";
    Image *image;
    pthread_t *threads;
    ThreadArgument *arguments;
    struct timespec start;
    struct timespec end;
    double elapsed;

    if (argc >= 2 && (!parse_int(argv[1], &thread_count) || thread_count < 1
                      || thread_count > 256)) {
        fprintf(stderr, "usage: %s [threads] [width] [height] [output.ppm]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 3 && (!parse_int(argv[2], &width) || width < 1 || width > 10000)) {
        fprintf(stderr, "invalid width\n");
        return EXIT_FAILURE;
    }
    if (argc >= 4 && (!parse_int(argv[3], &height) || height < 1 || height > 10000)) {
        fprintf(stderr, "invalid height\n");
        return EXIT_FAILURE;
    }
    if (argc >= 5) {
        output = argv[4];
    }

    image = image_new(width, height);
    canvas_width = width;
    canvas_height = height;
    threads = xmalloc((size_t)thread_count * sizeof(*threads));
    arguments = xmalloc((size_t)thread_count * sizeof(*arguments));
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int index = 0; index < thread_count; index++) {
        arguments[index].image = image;
        arguments[index].thread_index = index;
        arguments[index].thread_count = thread_count;
        if (pthread_create(&threads[index], NULL, worker, &arguments[index]) != 0) {
            die("pthread_create failed");
        }
    }
    for (int index = 0; index < thread_count; index++) {
        pthread_join(threads[index], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsed = (double)(end.tv_sec - start.tv_sec)
        + (double)(end.tv_nsec - start.tv_nsec) / 1e9;

    if (image_save(output, image) != 0) {
        die("cannot save %s", output);
    }
    printf("Fractale %dx%d, %d thread(s), %.3f s -> %s\n", width, height, thread_count,
           elapsed, output);
    image_free(image);
    free(arguments);
    free(threads);
    return EXIT_SUCCESS;
}

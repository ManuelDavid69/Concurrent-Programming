#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "gpu.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRIANGLE_CAPACITY 64
#define FRAGMENT_CAPACITY 8192

typedef struct {
    Triangle triangles[TRIANGLE_CAPACITY];
    int triangle_head;
    int triangle_tail;
    int triangle_count;
    Fragment fragments[FRAGMENT_CAPACITY];
    int fragment_head;
    int fragment_tail;
    int fragment_count;
    pthread_mutex_t mutex;
    pthread_cond_t triangle_not_empty;
    pthread_cond_t triangle_not_full;
    pthread_cond_t fragment_not_empty;
    pthread_cond_t fragment_not_full;
    pthread_cond_t sync_condition;
    pthread_barrier_t barrier;
    int epoch;
    int raster_done;
    int shader_done;
    int raster_busy;
    int raster_count;
    int shader_count;
    int done;
} Pipeline;

typedef struct {
    Pipeline *pipeline;
    int identifier;
} ThreadArgument;

static void push_triangle(Pipeline *pipeline, const Triangle *triangle)
{
    pthread_mutex_lock(&pipeline->mutex);
    while (pipeline->triangle_count == TRIANGLE_CAPACITY) {
        pthread_cond_wait(&pipeline->triangle_not_full, &pipeline->mutex);
    }
    pipeline->triangles[pipeline->triangle_tail] = *triangle;
    pipeline->triangle_tail = (pipeline->triangle_tail + 1) % TRIANGLE_CAPACITY;
    pipeline->triangle_count++;
    pthread_cond_signal(&pipeline->triangle_not_empty);
    pthread_mutex_unlock(&pipeline->mutex);
}

static int pop_triangle(Pipeline *pipeline, Triangle *triangle)
{
    if (pipeline->triangle_count == 0) {
        return 0;
    }
    *triangle = pipeline->triangles[pipeline->triangle_head];
    pipeline->triangle_head = (pipeline->triangle_head + 1) % TRIANGLE_CAPACITY;
    pipeline->triangle_count--;
    pthread_cond_signal(&pipeline->triangle_not_full);
    return 1;
}

static void push_fragment(Pipeline *pipeline, const Fragment *fragment)
{
    pthread_mutex_lock(&pipeline->mutex);
    while (pipeline->fragment_count == FRAGMENT_CAPACITY) {
        pthread_cond_wait(&pipeline->fragment_not_full, &pipeline->mutex);
    }
    pipeline->fragments[pipeline->fragment_tail] = *fragment;
    pipeline->fragment_tail = (pipeline->fragment_tail + 1) % FRAGMENT_CAPACITY;
    pipeline->fragment_count++;
    pthread_cond_signal(&pipeline->fragment_not_empty);
    pthread_mutex_unlock(&pipeline->mutex);
}

static int pop_fragment(Pipeline *pipeline, Fragment *fragment)
{
    if (pipeline->fragment_count == 0) {
        return 0;
    }
    *fragment = pipeline->fragments[pipeline->fragment_head];
    pipeline->fragment_head = (pipeline->fragment_head + 1) % FRAGMENT_CAPACITY;
    pipeline->fragment_count--;
    pthread_cond_signal(&pipeline->fragment_not_full);
    return 1;
}

static void rasterise_triangle(Pipeline *pipeline, const Triangle *triangle)
{
    void *raster = raster_init(triangle);
    Fragment fragment;

    while (raster_get_next_fragment(raster, &fragment)) {
        if (!fragment_is_sync(&fragment)) {
            push_fragment(pipeline, &fragment);
        }
    }
    raster_destroy(raster);
}

static void drain_rasters(Pipeline *pipeline)
{
    for (;;) {
        Triangle triangle;

        pthread_mutex_lock(&pipeline->mutex);
        if (pipeline->triangle_count == 0 && pipeline->raster_busy == 0) {
            pipeline->raster_done++;
            pthread_cond_broadcast(&pipeline->sync_condition);
            pthread_mutex_unlock(&pipeline->mutex);
            return;
        }
        if (pipeline->triangle_count > 0) {
            pop_triangle(pipeline, &triangle);
            pipeline->raster_busy++;
            pthread_mutex_unlock(&pipeline->mutex);
            rasterise_triangle(pipeline, &triangle);
            pthread_mutex_lock(&pipeline->mutex);
            pipeline->raster_busy--;
            pthread_cond_broadcast(&pipeline->sync_condition);
            pthread_mutex_unlock(&pipeline->mutex);
            continue;
        }
        pthread_cond_wait(&pipeline->sync_condition, &pipeline->mutex);
        pthread_mutex_unlock(&pipeline->mutex);
    }
}

static void *raster_stage(void *argument)
{
    ThreadArgument *thread = argument;
    Pipeline *pipeline = thread->pipeline;
    int my_epoch = 0;

    for (;;) {
        Triangle triangle;
        int epoch;

        pthread_mutex_lock(&pipeline->mutex);
        if (pipeline->epoch != my_epoch) {
            epoch = pipeline->epoch;
            pthread_mutex_unlock(&pipeline->mutex);
            drain_rasters(pipeline);
            pthread_barrier_wait(&pipeline->barrier);
            my_epoch = epoch;
            continue;
        }
        while (pipeline->triangle_count == 0 && pipeline->epoch == my_epoch
               && !pipeline->done) {
            pthread_cond_wait(&pipeline->triangle_not_empty, &pipeline->mutex);
        }
        if (pipeline->done && pipeline->triangle_count == 0) {
            pthread_mutex_unlock(&pipeline->mutex);
            return NULL;
        }
        if (pipeline->epoch != my_epoch) {
            epoch = pipeline->epoch;
            pthread_mutex_unlock(&pipeline->mutex);
            drain_rasters(pipeline);
            pthread_barrier_wait(&pipeline->barrier);
            my_epoch = epoch;
            continue;
        }
        if (!pop_triangle(pipeline, &triangle)) {
            pthread_mutex_unlock(&pipeline->mutex);
            continue;
        }
        pipeline->raster_busy++;
        pthread_mutex_unlock(&pipeline->mutex);
        rasterise_triangle(pipeline, &triangle);
        pthread_mutex_lock(&pipeline->mutex);
        pipeline->raster_busy--;
        pthread_cond_broadcast(&pipeline->sync_condition);
        pthread_mutex_unlock(&pipeline->mutex);
    }
}

static void drain_shaders(Pipeline *pipeline)
{
    for (;;) {
        Fragment fragment;

        pthread_mutex_lock(&pipeline->mutex);
        if (pipeline->fragment_count == 0 && pipeline->raster_done == pipeline->raster_count) {
            pipeline->shader_done++;
            pthread_cond_broadcast(&pipeline->sync_condition);
            pthread_mutex_unlock(&pipeline->mutex);
            return;
        }
        if (pipeline->fragment_count > 0) {
            pop_fragment(pipeline, &fragment);
            pthread_mutex_unlock(&pipeline->mutex);
            gpu_shader(fragment);
            continue;
        }
        pthread_cond_wait(&pipeline->sync_condition, &pipeline->mutex);
        pthread_mutex_unlock(&pipeline->mutex);
    }
}

static void *shader_stage(void *argument)
{
    ThreadArgument *thread = argument;
    Pipeline *pipeline = thread->pipeline;
    int my_epoch = 0;

    for (;;) {
        Fragment fragment;
        int epoch;

        pthread_mutex_lock(&pipeline->mutex);
        if (pipeline->epoch != my_epoch) {
            epoch = pipeline->epoch;
            pthread_mutex_unlock(&pipeline->mutex);
            drain_shaders(pipeline);
            pthread_barrier_wait(&pipeline->barrier);
            my_epoch = epoch;
            continue;
        }
        while (pipeline->fragment_count == 0 && pipeline->epoch == my_epoch
               && !pipeline->done) {
            pthread_cond_wait(&pipeline->fragment_not_empty, &pipeline->mutex);
        }
        if (pipeline->done && pipeline->fragment_count == 0) {
            pthread_mutex_unlock(&pipeline->mutex);
            return NULL;
        }
        if (pipeline->epoch != my_epoch) {
            epoch = pipeline->epoch;
            pthread_mutex_unlock(&pipeline->mutex);
            drain_shaders(pipeline);
            pthread_barrier_wait(&pipeline->barrier);
            my_epoch = epoch;
            continue;
        }
        if (!pop_fragment(pipeline, &fragment)) {
            pthread_mutex_unlock(&pipeline->mutex);
            continue;
        }
        pthread_mutex_unlock(&pipeline->mutex);
        gpu_shader(fragment);
    }
}

static void synchronise(Pipeline *pipeline)
{
    int epoch;

    pthread_mutex_lock(&pipeline->mutex);
    epoch = ++pipeline->epoch;
    pthread_cond_broadcast(&pipeline->sync_condition);
    pthread_cond_broadcast(&pipeline->triangle_not_empty);
    pthread_cond_broadcast(&pipeline->fragment_not_empty);
    pthread_mutex_unlock(&pipeline->mutex);

    pthread_barrier_wait(&pipeline->barrier);

    pthread_mutex_lock(&pipeline->mutex);
    pipeline->raster_done = 0;
    pipeline->shader_done = 0;
    pthread_mutex_unlock(&pipeline->mutex);
    (void)epoch;
}

int main(int argc, char **argv)
{
    int raster_count = 2;
    int shader_count = 2;
    const char *output = "pipeline.ppm";
    Pipeline pipeline = {0};
    void *creator;
    Triangle triangle;
    pthread_t *raster_threads;
    pthread_t *shader_threads;
    ThreadArgument *raster_arguments;
    ThreadArgument *shader_arguments;

    if (argc >= 2 && (!parse_int(argv[1], &raster_count) || raster_count < 1
                      || raster_count > 32)) {
        fprintf(stderr, "usage: %s [raster_threads] [shader_threads] [output.ppm]\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 3 && (!parse_int(argv[2], &shader_count) || shader_count < 1
                      || shader_count > 32)) {
        fprintf(stderr, "usage: %s [raster_threads] [shader_threads] [output.ppm]\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    if (argc >= 4) {
        output = argv[3];
    }
    setenv("PIPELINE_OUTPUT", output, 1);

    pipeline.raster_count = raster_count;
    pipeline.shader_count = shader_count;
    pthread_mutex_init(&pipeline.mutex, NULL);
    pthread_cond_init(&pipeline.triangle_not_empty, NULL);
    pthread_cond_init(&pipeline.triangle_not_full, NULL);
    pthread_cond_init(&pipeline.fragment_not_empty, NULL);
    pthread_cond_init(&pipeline.fragment_not_full, NULL);
    pthread_cond_init(&pipeline.sync_condition, NULL);
    pthread_barrier_init(&pipeline.barrier, NULL,
                         (unsigned int)(1 + raster_count + shader_count));

    raster_threads = xmalloc((size_t)raster_count * sizeof(*raster_threads));
    shader_threads = xmalloc((size_t)shader_count * sizeof(*shader_threads));
    raster_arguments = xmalloc((size_t)raster_count * sizeof(*raster_arguments));
    shader_arguments = xmalloc((size_t)shader_count * sizeof(*shader_arguments));

    for (int index = 0; index < raster_count; index++) {
        raster_arguments[index].pipeline = &pipeline;
        raster_arguments[index].identifier = index;
        pthread_create(&raster_threads[index], NULL, raster_stage, &raster_arguments[index]);
    }
    for (int index = 0; index < shader_count; index++) {
        shader_arguments[index].pipeline = &pipeline;
        shader_arguments[index].identifier = index;
        pthread_create(&shader_threads[index], NULL, shader_stage, &shader_arguments[index]);
    }

    creator = triangle_creator_init();
    while (triangle_creator_get_next_triangle(creator, &triangle)) {
        if (triangle_is_sync(&triangle)) {
            synchronise(&pipeline);
        } else {
            push_triangle(&pipeline, &triangle);
        }
    }
    synchronise(&pipeline);

    pthread_mutex_lock(&pipeline.mutex);
    pipeline.done = 1;
    pthread_cond_broadcast(&pipeline.triangle_not_empty);
    pthread_cond_broadcast(&pipeline.fragment_not_empty);
    pthread_mutex_unlock(&pipeline.mutex);

    for (int index = 0; index < raster_count; index++) {
        pthread_join(raster_threads[index], NULL);
    }
    for (int index = 0; index < shader_count; index++) {
        pthread_join(shader_threads[index], NULL);
    }
    triangle_creator_destroy(creator);
    printf("Pipeline terminé avec %d rasteriseur(s) et %d shader(s).\n",
           raster_count, shader_count);

    pthread_barrier_destroy(&pipeline.barrier);
    pthread_cond_destroy(&pipeline.sync_condition);
    pthread_cond_destroy(&pipeline.fragment_not_full);
    pthread_cond_destroy(&pipeline.fragment_not_empty);
    pthread_cond_destroy(&pipeline.triangle_not_full);
    pthread_cond_destroy(&pipeline.triangle_not_empty);
    pthread_mutex_destroy(&pipeline.mutex);
    free(shader_arguments);
    free(raster_arguments);
    free(shader_threads);
    free(raster_threads);
    return EXIT_SUCCESS;
}

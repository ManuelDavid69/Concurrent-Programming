#define _POSIX_C_SOURCE 200809L

#include "gpu.h"
#include "common.h"
#include "image.h"

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 640
#define HEIGHT 480
#define TORUS_U 28
#define TORUS_V 14
#define BAND_TRIANGLES 112
#define PI 3.14159265358979323846

typedef struct {
    Triangle *triangles;
    int count;
    int capacity;
    int index;
    Image *image;
    float *depth;
} Creator;

static pthread_mutex_t shader_mutex = PTHREAD_MUTEX_INITIALIZER;
static Image *gpu_image = NULL;
static float *gpu_depth = NULL;

static const unsigned char palette[6][3] = {
    {230, 70, 55},
    {240, 180, 60},
    {130, 205, 90},
    {55, 185, 220},
    {95, 120, 235},
    {225, 95, 205},
};

static void add_triangle(Creator *creator, Vertex first, Vertex second, Vertex third,
                         const unsigned char color[3], int sync)
{
    Triangle *triangle;

    if (creator->count == creator->capacity) {
        creator->capacity = creator->capacity == 0 ? 256 : creator->capacity * 2;
        creator->triangles = xrealloc(creator->triangles,
                                      (size_t)creator->capacity * sizeof(*creator->triangles));
    }
    triangle = &creator->triangles[creator->count++];
    triangle->vertices[0] = first;
    triangle->vertices[1] = second;
    triangle->vertices[2] = third;
    triangle->color[0] = color[0];
    triangle->color[1] = color[1];
    triangle->color[2] = color[2];
    triangle->sync = sync;
}

static Vertex project(double x, double y, double z)
{
    const double angle_y = 0.65;
    const double angle_x = -0.35;
    double x1 = x * cos(angle_y) + z * sin(angle_y);
    double z1 = -x * sin(angle_y) + z * cos(angle_y);
    double y1 = y * cos(angle_x) - z1 * sin(angle_x);
    double z2 = y * sin(angle_x) + z1 * cos(angle_x);
    const double scale = 170.0;
    Vertex vertex;

    vertex.x = (float)(WIDTH / 2.0 + x1 * scale);
    vertex.y = (float)(HEIGHT / 2.0 - y1 * scale);
    vertex.z = (float)z2;
    return vertex;
}

static Vertex torus_point(double u, double v)
{
    const double major = 1.0;
    const double minor = 0.42;
    double radius = major + minor * cos(v);

    return project(radius * cos(u), radius * sin(u), minor * sin(v));
}

void *triangle_creator_init(void)
{
    Creator *creator = xcalloc(1, sizeof(*creator));
    unsigned char sync_color[3] = {0, 0, 0};
    int normal_count = 0;

    creator->image = image_new(WIDTH, HEIGHT);
    creator->depth = xmalloc((size_t)WIDTH * (size_t)HEIGHT * sizeof(*creator->depth));
    if (creator->image == NULL) {
        die("cannot create the pipeline image");
    }
    gpu_image = creator->image;
    gpu_depth = creator->depth;
    for (int index = 0; index < WIDTH * HEIGHT; index++) {
        creator->depth[index] = 1e30f;
        creator->image->pixels[3 * index] = 18;
        creator->image->pixels[3 * index + 1] = 20;
        creator->image->pixels[3 * index + 2] = 28;
    }

    for (int u_index = 0; u_index < TORUS_U; u_index++) {
        double u0 = 2.0 * PI * u_index / TORUS_U;
        double u1 = 2.0 * PI * (u_index + 1) / TORUS_U;
        int band = (normal_count / BAND_TRIANGLES) % 6;

        for (int v_index = 0; v_index < TORUS_V; v_index++) {
            double v0 = 2.0 * PI * v_index / TORUS_V;
            double v1 = 2.0 * PI * (v_index + 1) / TORUS_V;
            Vertex p00 = torus_point(u0, v0);
            Vertex p10 = torus_point(u1, v0);
            Vertex p11 = torus_point(u1, v1);
            Vertex p01 = torus_point(u0, v1);

            add_triangle(creator, p00, p10, p11, palette[band], 0);
            add_triangle(creator, p00, p11, p01, palette[band], 0);
            normal_count += 2;
            if (normal_count % BAND_TRIANGLES == 0) {
                add_triangle(creator, p00, p10, p11, sync_color, 1);
            }
        }
    }
    return creator;
}

int triangle_creator_get_next_triangle(void *opaque, struct Triangle *triangle)
{
    Creator *creator = opaque;

    if (creator->index >= creator->count) {
        return 0;
    }
    *triangle = creator->triangles[creator->index++];
    return 1;
}

int triangle_is_sync(struct Triangle *triangle)
{
    return triangle->sync;
}

void triangle_creator_destroy(void *opaque)
{
    Creator *creator = opaque;
    const char *output = getenv("PIPELINE_OUTPUT");

    if (output == NULL || *output == '\0') {
        output = "pipeline.ppm";
    }
    if (image_save(output, creator->image) != 0) {
        fprintf(stderr, "cannot save %s\n", output);
    } else {
        printf("Image enregistrée: %s (%dx%d)\n", output, WIDTH, HEIGHT);
    }
    free(creator->depth);
    image_free(creator->image);
    gpu_image = NULL;
    gpu_depth = NULL;
    free(creator->triangles);
    free(creator);
}

typedef struct {
    Triangle triangle;
    int min_x;
    int max_x;
    int min_y;
    int max_y;
    int x;
    int y;
    int sync_done;
    double area;
    int valid;
} RasterState;

static double edge(Vertex first, Vertex second, double x, double y)
{
    return (second.x - first.x) * (y - first.y) - (second.y - first.y) * (x - first.x);
}

void *raster_init(const struct Triangle *triangle)
{
    RasterState *state = xcalloc(1, sizeof(*state));

    state->triangle = *triangle;
    if (triangle->sync) {
        return state;
    }
    state->min_x = (int)floor(fminf(triangle->vertices[0].x,
                                    fminf(triangle->vertices[1].x, triangle->vertices[2].x)));
    state->max_x = (int)ceilf(fmaxf(triangle->vertices[0].x,
                                    fmaxf(triangle->vertices[1].x, triangle->vertices[2].x)));
    state->min_y = (int)floor(fminf(triangle->vertices[0].y,
                                    fminf(triangle->vertices[1].y, triangle->vertices[2].y)));
    state->max_y = (int)ceilf(fmaxf(triangle->vertices[0].y,
                                    fmaxf(triangle->vertices[1].y, triangle->vertices[2].y)));
    if (state->min_x < 0) {
        state->min_x = 0;
    }
    if (state->min_y < 0) {
        state->min_y = 0;
    }
    if (state->max_x >= WIDTH) {
        state->max_x = WIDTH - 1;
    }
    if (state->max_y >= HEIGHT) {
        state->max_y = HEIGHT - 1;
    }
    state->area = edge(triangle->vertices[0], triangle->vertices[1],
                       triangle->vertices[2].x, triangle->vertices[2].y);
    state->valid = fabs(state->area) > 1e-9 && state->min_x <= state->max_x
        && state->min_y <= state->max_y;
    state->x = state->min_x;
    state->y = state->min_y;
    return state;
}

struct Fragment fragment_sync(void)
{
    Fragment fragment;

    memset(&fragment, 0, sizeof(fragment));
    fragment.sync = 1;
    return fragment;
}

int fragment_is_sync(struct Fragment *fragment)
{
    return fragment->sync;
}

int raster_get_next_fragment(void *opaque, struct Fragment *fragment)
{
    RasterState *state = opaque;
    Triangle *triangle = &state->triangle;

    if (triangle->sync) {
        if (state->sync_done) {
            return 0;
        }
        state->sync_done = 1;
        *fragment = fragment_sync();
        return 1;
    }
    if (!state->valid) {
        return 0;
    }
    while (state->y <= state->max_y) {
        while (state->x <= state->max_x) {
            double x = state->x + 0.5;
            double y = state->y + 0.5;
            double w0 = edge(triangle->vertices[1], triangle->vertices[2], x, y) / state->area;
            double w1 = edge(triangle->vertices[2], triangle->vertices[0], x, y) / state->area;
            double w2 = 1.0 - w0 - w1;

            state->x++;
            if (w0 >= -1e-6 && w1 >= -1e-6 && w2 >= -1e-6) {
                fragment->x = state->x - 1;
                fragment->y = state->y;
                fragment->color[0] = triangle->color[0];
                fragment->color[1] = triangle->color[1];
                fragment->color[2] = triangle->color[2];
                fragment->depth = (float)(w0 * triangle->vertices[0].z
                                          + w1 * triangle->vertices[1].z
                                          + w2 * triangle->vertices[2].z);
                fragment->sync = 0;
                return 1;
            }
        }
        state->x = state->min_x;
        state->y++;
    }
    return 0;
}

void raster_destroy(void *opaque)
{
    free(opaque);
}

void gpu_shader(struct Fragment fragment)
{
    int index;

    if (fragment.sync || fragment.x < 0 || fragment.x >= WIDTH || fragment.y < 0
        || fragment.y >= HEIGHT || gpu_image == NULL || gpu_depth == NULL) {
        return;
    }
    index = fragment.y * WIDTH + fragment.x;
    pthread_mutex_lock(&shader_mutex);
    if (fragment.depth < gpu_depth[index]) {
        gpu_depth[index] = fragment.depth;
        gpu_image->pixels[3 * index] = fragment.color[0];
        gpu_image->pixels[3 * index + 1] = fragment.color[1];
        gpu_image->pixels[3 * index + 2] = fragment.color[2];
    }
    pthread_mutex_unlock(&shader_mutex);
}

#ifndef PIPELINE_GPU_H
#define PIPELINE_GPU_H

typedef struct {
    float x;
    float y;
    float z;
} Vertex;

struct Triangle {
    Vertex vertices[3];
    unsigned char color[3];
    int sync;
};
typedef struct Triangle Triangle;

struct Fragment {
    int x;
    int y;
    unsigned char color[3];
    float depth;
    int sync;
};
typedef struct Fragment Fragment;

/*
 * The three functions below are the only entry points of the graphics
 * library.  triangle_creator_init() loads a procedural 3D scene and clears
 * the image; destroy() saves the image to PIPELINE_OUTPUT or pipeline.ppm.
 */
void *triangle_creator_init(void);
int triangle_creator_get_next_triangle(void *creator, struct Triangle *triangle);
void triangle_creator_destroy(void *creator);
int triangle_is_sync(struct Triangle *triangle);

void *raster_init(const struct Triangle *triangle);
int raster_get_next_fragment(void *raster_state, struct Fragment *fragment);
void raster_destroy(void *raster_state);
struct Fragment fragment_sync(void);
int fragment_is_sync(struct Fragment *fragment);

void gpu_shader(struct Fragment fragment);

#endif

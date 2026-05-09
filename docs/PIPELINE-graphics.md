# Graphics pipeline — threads and barriers

## Objectives

- build a multi-stage producer/consumer pipeline;
- connect stages with bounded buffers;
- synchronise all stages with a barrier when the scene changes;
- render a complete PPM image from a procedural 3D scene.

---

## 1. Architecture

```
                 triangles                     fragments
creator thread -----------> raster threads -----------> shader threads --> image
                buffer                        buffer
```

| Stage | Threads | Input | Output |
|---|---|---|---|
| triangle creator | 1 | procedural torus | `Triangle` buffer |
| rasteriser | R | triangles | `Fragment` buffer |
| shader | S | fragments | image + z-buffer |

`R` and `S` are command-line arguments.  The queues are bounded to keep
memory usage constant and to apply back-pressure to the producer.

`gpu.c` provides the graphics primitives and is not modified:

```c
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
```

`triangle_creator_get_next_triangle` returns 0 at the end of the scene.
`triangle_is_sync` identifies a colour-change marker.  `raster_init` followed
by repeated `raster_get_next_fragment` enumerates the pixels covered by a
triangle (one fragment per pixel).  `gpu_shader` performs the z-test and writes
the pixel.

---

## 2. Procedural scene

The scene is a torus generated with 28 × 14 quads, i.e. 784 triangles.  Every
112 triangles the colour changes and a synchronisation triangle is inserted.
The torus is rotated around the y and x axes and projected orthographically
onto a 640×480 image.

The six palette colours and the dark background explain why the final image
contains only seven distinct RGB values: the shader uses flat colours, not
interpolated lighting.  The point of the lab is the pipeline, not a physically
based renderer.

---

## 3. Pipeline synchronisation

### Queues

Both buffers are circular arrays protected by one mutex and two condition
variables per queue (`not_empty`, `not_full`).  A producer waits when the queue
is full; a consumer waits when it is empty.

### Colour-change barriers

A plain producer/consumer pipeline has no reason to wait: fragments from the
old colour can still be in the buffers when the creator starts producing the
new colour, which produces visible tearing.  The synchronisation triangle
triggers a **global drain**:

1. the creator increments `epoch`, broadcasts every condition and waits on a
   barrier of `1 + R + S` threads;
2. every raster thread notices the new epoch, stops taking new triangles,
   finishes the triangles already in flight (`raster_busy`), drains the
   triangle queue, increments `raster_done`, then waits at the barrier;
3. every shader thread notices the new epoch, keeps consuming fragments until
   `raster_done == R` and the fragment queue is empty, increments
   `shader_done`, then waits at the barrier;
4. when all threads have arrived, the barrier releases them and the creator
   resets the counters and continues with the new colour.

The `raster_busy` counter is essential: a triangle may have been dequeued but
not yet fully rasterised when the epoch changes.  Counting it prevents the
shaders from declaring the pipeline empty too early.

Using a barrier (rather than a mutex/condition counter) also makes the
rendezvous reusable: every thread calls `pthread_barrier_wait` exactly once per
colour change, and the barrier automatically resets for the next round.

### End of the scene

After the last triangle, the creator performs one final synchronisation.  This
guarantees that every fragment has been shaded before
`triangle_creator_destroy` saves the image.  Then the `done` flag is set, the
conditions are broadcast, and all threads are joined.

---

## 4. Rasterisation and shading

For each triangle, `raster_init` computes the bounding box and the signed area.
For each pixel centre inside the triangle, barycentric weights give the colour
and the interpolated depth.  `gpu_shader` compares the depth with the z-buffer
under a mutex and writes the pixel if it is closer.

The z-buffer is the only shared structure written by the shaders; the mutex
makes the compare-and-write atomic.  The image itself is written at the same
time as the depth, so no other lock is needed.

---

## 5. Results

- `pipeline 1 1` — minimal configuration, easier to debug;
- `pipeline 2 2` — the default;
- `pipeline 4 3` — more parallelism.

All configurations produce a 640×480 P6 image with the same seven colours and
the same torus silhouette.  The exact pixel values are deterministic for a
given configuration; different thread counts may differ on shared triangle
edges because the order of equal-depth fragments is not specified.

The generated image is shown in the repository's README when the pipeline is
run.  The barrier removes the horizontal colour bands that appear when the
stages are not synchronised.

---

## Build, run and test

```bash
make -C pipeline
./pipeline/bin/pipeline 2 2 pipeline.ppm
./pipeline/bin/pipeline 1 1 small.ppm
python3 -m pytest tests/test_pipeline.py -q
```

The tests run the pipeline with 1/1, 2/2 and 4/3 threads, check the PPM
dimensions, the number of colours and the number of non-background pixels, and
verify that a zero thread count is rejected.

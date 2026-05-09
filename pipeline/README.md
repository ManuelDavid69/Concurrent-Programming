# Graphics pipeline — threads and barriers

A software rendering pipeline with three stages:

```
triangle creator -> triangle buffer -> rasterisers -> fragment buffer -> shaders -> image
```

```bash
make
./bin/pipeline 2 2 pipeline.ppm
./bin/pipeline 1 1 small.ppm
```

- one triangle-creator thread (the main thread) generates a procedural torus;
- several rasteriser threads turn triangles into fragments;
- several shader threads apply the z-buffer and write the pixels;
- bounded queues connect the stages.

Whenever the scene changes colour, the creator inserts a synchronisation
point.  All stages drain their queues and meet at a `pthread_barrier_t`, which
guarantees that no triangle or fragment from the previous colour is shaded
after the change.  This is what removes the tearing visible in the
unsynchronised version.

`gpu.c` contains the graphics primitives (scene generation, rasterisation and
shading) and must not be modified; only `pipeline.c` contains the
synchronisation logic.

The output is a 640×480 P6 PPM file.  The number of rasteriser and shader
threads is configurable; both must be at least one.

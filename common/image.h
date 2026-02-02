#ifndef PC_IMAGE_H
#define PC_IMAGE_H

#include <stddef.h>

/*
 * Minimal binary/ASCII PPM (P6/P3) image.  Pixels are stored as RGB
 * triplets in a single one-dimensional array, exactly like the course
 * statement suggests: offset = 3 * (y * width + x).
 */
typedef struct {
    int width;
    int height;
    unsigned char *pixels;
} Image;

Image *image_new(int width, int height);
void image_free(Image *image);
Image *image_load(const char *path);
int image_save(const char *path, const Image *image);

unsigned char *image_pixel(Image *image, int x, int y);
const unsigned char *image_pixel_const(const Image *image, int x, int y);

/* 3x3 mean blur; pixels outside the image are ignored (kernel crop). */
Image *image_blur_crop(const Image *source);

#endif

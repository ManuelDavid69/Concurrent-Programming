#ifndef PC_IMAGE_H
#define PC_IMAGE_H

#include <stddef.h>

/*
 * Image PPM (P6/P3) binaire/ASCII minimale.  Les pixels sont stockés sous
 * forme de triplets RGB dans un unique tableau à une dimension, exactement
 * comme le suggère l'énoncé du cours : offset = 3 * (y * width + x).
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

/* Flou moyen 3x3 ; les pixels hors de l'image sont ignorés (recadrage du noyau). */
Image *image_blur_crop(const Image *source);

#endif

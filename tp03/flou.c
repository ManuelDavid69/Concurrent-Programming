#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    Image *source;
    Image *blurred;
    const unsigned char *pixel;

    if (argc != 3) {
        fprintf(stderr, "usage: %s input.ppm output.ppm\n", argv[0]);
        return EXIT_FAILURE;
    }
    source = image_load(argv[1]);
    if (source == NULL) {
        return EXIT_FAILURE;
    }
    blurred = image_blur_crop(source);
    if (blurred == NULL) {
        fprintf(stderr, "cannot allocate the output image\n");
        image_free(source);
        return EXIT_FAILURE;
    }
    if (image_save(argv[2], blurred) != 0) {
        fprintf(stderr, "cannot save %s\n", argv[2]);
        image_free(source);
        image_free(blurred);
        return EXIT_FAILURE;
    }

    pixel = image_pixel_const(blurred, source->width / 2, source->height / 2);
    printf("Image %dx%d traitée (%s -> %s), pixel central: (%u, %u, %u)\n",
           source->width, source->height, argv[1], argv[2], pixel[0], pixel[1], pixel[2]);
    image_free(source);
    image_free(blurred);
    return EXIT_SUCCESS;
}

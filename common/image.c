#define _POSIX_C_SOURCE 200809L

#include "image.h"
#include "common.h"

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int skip_space_and_comments(FILE *file)
{
    int character;

    for (;;) {
        character = fgetc(file);
        if (character == '#') {
            while (character != '\n' && character != EOF) {
                character = fgetc(file);
            }
        } else if (isspace(character)) {
            continue;
        } else {
            return character;
        }
    }
}

static int read_token(FILE *file, char *buffer, size_t size)
{
    int character;
    size_t length = 0;

    character = skip_space_and_comments(file);
    if (character == EOF) {
        return -1;
    }
    while (character != EOF && !isspace(character)) {
        if (length + 1 >= size) {
            return -1;
        }
        buffer[length++] = (char)character;
        character = fgetc(file);
    }
    buffer[length] = '\0';
    return 0;
}

Image *image_new(int width, int height)
{
    Image *image;

    if (width <= 0 || height <= 0) {
        return NULL;
    }
    if ((size_t)width > SIZE_MAX / 3 / (size_t)height) {
        return NULL;
    }
    image = xmalloc(sizeof(*image));
    image->width = width;
    image->height = height;
    image->pixels = xcalloc((size_t)width * (size_t)height * 3, 1);
    return image;
}

void image_free(Image *image)
{
    if (image != NULL) {
        free(image->pixels);
        free(image);
    }
}

unsigned char *image_pixel(Image *image, int x, int y)
{
    return image->pixels + 3 * ((size_t)y * (size_t)image->width + (size_t)x);
}

const unsigned char *image_pixel_const(const Image *image, int x, int y)
{
    return image->pixels + 3 * ((size_t)y * (size_t)image->width + (size_t)x);
}

static bool parse_dimension(const char *text, int *value)
{
    long parsed;

    if (!parse_long(text, &parsed) || parsed <= 0 || parsed > 100000) {
        return false;
    }
    *value = (int)parsed;
    return true;
}

Image *image_load(const char *path)
{
    FILE *file;
    char token[64];
    bool binary;
    int width;
    int height;
    int maximum;
    Image *image;

    file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "cannot open %s: %s\n", path, strerror(errno));
        return NULL;
    }
    if (read_token(file, token, sizeof(token)) != 0
        || (strcmp(token, "P6") != 0 && strcmp(token, "P3") != 0)) {
        fprintf(stderr, "%s: not a P3/P6 PPM file\n", path);
        fclose(file);
        return NULL;
    }
    binary = strcmp(token, "P6") == 0;
    if (read_token(file, token, sizeof(token)) != 0
        || !parse_dimension(token, &width)) {
        fprintf(stderr, "%s: invalid width\n", path);
        fclose(file);
        return NULL;
    }
    if (read_token(file, token, sizeof(token)) != 0
        || !parse_dimension(token, &height)) {
        fprintf(stderr, "%s: invalid height\n", path);
        fclose(file);
        return NULL;
    }
    if (read_token(file, token, sizeof(token)) != 0) {
        fprintf(stderr, "%s: missing maximum value\n", path);
        fclose(file);
        return NULL;
    }
    if (!parse_int(token, &maximum) || maximum != 255) {
        fprintf(stderr, "%s: only 8-bit PPM files are supported\n", path);
        fclose(file);
        return NULL;
    }
    image = image_new(width, height);
    if (image == NULL) {
        fprintf(stderr, "%s: image is too large\n", path);
        fclose(file);
        return NULL;
    }
    if (binary) {
        size_t count = (size_t)width * (size_t)height * 3;
        if (fread(image->pixels, 1, count, file) != count) {
            fprintf(stderr, "%s: truncated pixel data\n", path);
            image_free(image);
            fclose(file);
            return NULL;
        }
    } else {
        size_t count = (size_t)width * (size_t)height * 3;
        size_t index;
        for (index = 0; index < count; index++) {
            int value;
            if (read_token(file, token, sizeof(token)) != 0
                || !parse_int(token, &value) || value < 0 || value > 255) {
                fprintf(stderr, "%s: invalid P3 pixel\n", path);
                image_free(image);
                fclose(file);
                return NULL;
            }
            image->pixels[index] = (unsigned char)value;
        }
    }
    fclose(file);
    return image;
}

int image_save(const char *path, const Image *image)
{
    FILE *file;
    size_t count;

    file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "cannot create %s: %s\n", path, strerror(errno));
        return -1;
    }
    if (fprintf(file, "P6\n%d %d\n255\n", image->width, image->height) < 0) {
        fclose(file);
        return -1;
    }
    count = (size_t)image->width * (size_t)image->height * 3;
    if (fwrite(image->pixels, 1, count, file) != count) {
        fclose(file);
        return -1;
    }
    if (fclose(file) != 0) {
        return -1;
    }
    return 0;
}

Image *image_blur_crop(const Image *source)
{
    Image *result = image_new(source->width, source->height);
    int x;
    int y;

    if (result == NULL) {
        return NULL;
    }
    for (y = 0; y < source->height; y++) {
        for (x = 0; x < source->width; x++) {
            int sum_red = 0;
            int sum_green = 0;
            int sum_blue = 0;
            int count = 0;
            int dy;
            int dx;

            for (dy = -1; dy <= 1; dy++) {
                for (dx = -1; dx <= 1; dx++) {
                    int neighbour_x = x + dx;
                    int neighbour_y = y + dy;
                    const unsigned char *pixel;

                    if (neighbour_x < 0 || neighbour_x >= source->width
                        || neighbour_y < 0 || neighbour_y >= source->height) {
                        continue;
                    }
                    pixel = image_pixel_const(source, neighbour_x, neighbour_y);
                    sum_red += pixel[0];
                    sum_green += pixel[1];
                    sum_blue += pixel[2];
                    count++;
                }
            }
            result->pixels[3 * ((size_t)y * (size_t)source->width + (size_t)x)] =
                (unsigned char)(sum_red / count);
            result->pixels[3 * ((size_t)y * (size_t)source->width + (size_t)x) + 1] =
                (unsigned char)(sum_green / count);
            result->pixels[3 * ((size_t)y * (size_t)source->width + (size_t)x) + 2] =
                (unsigned char)(sum_blue / count);
        }
    }
    return result;
}

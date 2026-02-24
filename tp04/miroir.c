#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void reverse(char *text)
{
    size_t left = 0;
    size_t right = strlen(text);

    if (right == 0) {
        return;
    }
    right--;
    while (left < right) {
        char temporary = text[left];
        text[left] = text[right];
        text[right] = temporary;
        left++;
        right--;
    }
}

int main(int argc, char **argv)
{
    int index;

    if (argc < 2) {
        fprintf(stderr, "usage: %s word [word ...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    for (index = 1; index < argc; index++) {
        reverse(argv[index]);
        printf("%s%s", index == 1 ? "" : " ", argv[index]);
    }
    printf("\n");
    return EXIT_SUCCESS;
}

#define _POSIX_C_SOURCE 200809L

#include "automate.h"

#include <string.h>

bool automate_init(Automate *automate, const char *code)
{
    size_t length = strlen(code);
    int index;
    int border = 0;

    if (length == 0 || length >= AUTOMATE_MAX_CODE) {
        return false;
    }
    memcpy(automate->code, code, length + 1);
    automate->length = (int)length;
    automate->state = 0;
    automate->prefix[0] = 0;
    for (index = 1; index < automate->length; index++) {
        while (border > 0 && code[index] != code[border]) {
            border = automate->prefix[border - 1];
        }
        if (code[index] == code[border]) {
            border++;
        }
        automate->prefix[index] = border;
    }
    return true;
}

void automate_reset(Automate *automate)
{
    automate->state = 0;
}

int automate_feed(Automate *automate, char character)
{
    while (automate->state > 0 && character != automate->code[automate->state]) {
        automate->state = automate->prefix[automate->state - 1];
    }
    if (character == automate->code[automate->state]) {
        automate->state++;
    }
    if (automate->state == automate->length) {
        automate->state = 0;
        return 1;
    }
    return 0;
}

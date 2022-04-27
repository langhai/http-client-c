

#ifndef HTTP_CLIENT_C_HTTP_CLIENT_HEX_H
#define HTTP_CLIENT_C_HTTP_CLIENT_HEX_H

#include "stddef.h"
#include "string.h"
#include <math.h>

size_t hex2dec(const char *, size_t len);
void str_to_upper(char *, size_t);

char *str2hex(unsigned char *str, size_t len) {

    char *result = malloc(2 *  len + 1);

    for (size_t i = 0; i < len; ++i) {

        sprintf(&result[2 * i], "%02X", str[i]);
    }

    result[2 * len] = '\0';
    return result;
}

char *hex2str(char *str, size_t *len) {

    size_t counter = 0;
    size_t size = strlen(str);
    char *result = malloc(*len / 2 + 1);

    for (size_t i = 0; i < size; i += 2) {

        result[counter++] = (char) hex2dec(&str[i], 2);
    }

    *len = counter - 1;
    result[counter] = '\0';
    return result;
}

size_t hex2dec(const char *hex, size_t len) {

    size_t i = len;
    size_t j = i - 1;

    size_t result = 0;

    while (i-- > 0) {

        char ch = hex[i];

        if (ch >= '0' && ch <= '9') {

            result += (ch - '0') * pow(16, j - i);
        } else {

            if (ch >= 'a') {

                ch -= 32;
            }

            if (ch >= 'A' && ch <= 'F') {

                result += (ch - 'A' + 10) * pow(16, j - i);
            }
        }
    }

    return result;
}

void str_to_upper(char *str, size_t len) {

    char c;

    for (int i = 0; i < len; i++) {

        c = str[i];

        if (c >= 'a' && c <= 'z') {

            str[i] = c - 32;
        }
    }
}

#endif

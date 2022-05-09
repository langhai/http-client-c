

#ifndef HTTP_CLIENT_C_ENCODING_STRUCT_H
#define HTTP_CLIENT_C_ENCODING_STRUCT_H

//#include <ctype.h>
//#include <math.h>
//#include "stdio.h"
#include "string.h"
#include "error.h"

typedef struct http_transfer_encoding {

    char *value;
    struct http_transfer_encoding *next;

} http_transfer_encoding;

http_transfer_encoding *http_transfer_encoding_new();
http_transfer_encoding *http_transfer_encoding_parse(const char *);
void http_transfer_encoding_set_value(http_transfer_encoding *, const char *);
void http_transfer_encoding_free(http_transfer_encoding *);

//http_client_errors http_transfer_decode(http_transfer_encoding *te, int socket, const char *buf, size_t buf_len, http_response_body_cb_ptr *);

//
http_transfer_encoding *http_transfer_encoding_new() {

    http_transfer_encoding *te = (http_transfer_encoding *) calloc(sizeof (http_transfer_encoding), 1);

    return te;
}

http_transfer_encoding *http_transfer_encoding_parse(const char *transfer_encoding) {

    http_transfer_encoding *te, *tmp;

    te = tmp = NULL;

    char *encode = strdup(transfer_encoding);

    char *token = NULL;

    while ((token = strsep(&encode, " \t\r\n,")) != NULL) {

        if (strcmp(token, "") == 0) {

            continue;
        }

        tmp = http_transfer_encoding_new();
        http_transfer_encoding_set_value(tmp, token);

        if (te == NULL) {

            te = tmp;
        }

        else {

            tmp->next = te;
            te = tmp;
        }
    }

    return te;
}

http_transfer_encoding *http_transfer_encoding_clone(const http_transfer_encoding *transfer_encoding) {

    http_transfer_encoding *te = NULL, *tmp3, *tmp2, *tmp = (http_transfer_encoding *) transfer_encoding;

    while (tmp != NULL) {

        if (te == NULL) {

            tmp2 = te = http_transfer_encoding_new();
            http_transfer_encoding_set_value(te, tmp->value);
        }

        else {

            tmp3 = http_transfer_encoding_new();
            http_transfer_encoding_set_value(tmp3, tmp->value);

            tmp2->next = tmp3;
            tmp2 = tmp2->next;
        }

        tmp = tmp->next;
    }

    return te;
}

void http_transfer_encoding_set_value(http_transfer_encoding *te, const char *value) {

    if (te->value != NULL) {

        free(te->value);
        te->value = NULL;
    }

    if(value != NULL) {

        te->value = strdup(value);
    }
}

void http_transfer_encoding_free(http_transfer_encoding *te) {

    if (te != NULL) {

        if (te->value != NULL) {

            free(te->value);
            te->value = NULL;
        }

        if (te->next != NULL) {

            http_transfer_encoding_free(te->next);
            te->next = NULL;
        }

        free(te);
    }
}

#endif

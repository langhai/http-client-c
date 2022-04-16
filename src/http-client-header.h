//
// Created by tbela on 2022-04-08.
//

#ifndef HTTP_CLIENT_C_HTTP_CLIENT_HEADER_H
#define HTTP_CLIENT_C_HTTP_CLIENT_HEADER_H

#include "http-client-struct.h"
//#include "http-client-c.h"

struct http_header *http_header_new();

struct http_header *http_header_clone(struct http_header *);

int http_header_exists(struct http_header *header, const char *name);

struct http_header *http_header_get(struct http_header *header, const char *name);

void http_header_set(struct http_request *hreq, const char *name, const char *value);

void http_header_add(struct http_request *hreq, const char *name, const char *value);

void http_header_unset(struct http_request *hreq, const char *name);

void http_header_set_name(struct http_header *header, char *name);

void http_header_set_value(struct http_header *header, char *value);

//char *http_headers_print(const struct http_header *header);

struct http_header *http_header_parse(const char *http_headers, size_t *len);

void http_header_free(struct http_header *header);

// def

struct http_header *http_header_new() {

    struct http_header *header = (struct http_header *) calloc(sizeof(struct http_header), 1);
//    memset(header, 0, sizeof(struct http_header));

    return header;
}

struct http_header *http_header_clone(struct http_header *headers) {

    if (headers == NULL) {

        return NULL;
    }

    struct http_header *header = http_header_new();
    struct http_header *next = header;
    struct http_header *current = headers;

    while (current != NULL) {

        http_header_set_name(next, current->name);
        http_header_set_value(next, current->value);

        current = current->next;

        if (current != NULL) {

            next->next = http_header_new();
            next = next->next;
        }
    }

    return header;
}

int http_header_exists(struct http_header *header, const char *name) {

    struct http_header *head = header;

    while (head != NULL) {

        if (strcasecmp(head->name, name) == 0) {

            return 1;
        }

        head = head->next;
    }

    return 0;
}

struct http_header *http_header_get(struct http_header *header, const char *name) {

    struct http_header *head = header;
    struct http_header *result = NULL;
    struct http_header *tmp, *tmp2;

    size_t len;

    while (head != NULL) {

        if (strcasecmp(head->name, name) == 0) {

            if (result == NULL) {

                result = http_header_new();

                http_header_set_name(result, head->name);
                http_header_set_value(result, head->value);

                tmp = result;
            } else {

                tmp2 = http_header_new();

                http_header_set_name(tmp2, head->name);
                http_header_set_value(tmp2, head->value);

                tmp->next = tmp2;
                tmp = tmp2;
            }
        }

        head = head->next;
    }

    return result;
}

void http_header_set(struct http_request *hreq, const char *name, const char *value) {

    struct http_header *header = hreq->headers;

    if (header == NULL) {

        hreq->headers = http_header_new();

        http_header_set_name(hreq->headers, (char *) name);
        http_header_set_value(hreq->headers, (char *) value);
        return;
    }

    if (strcasecmp(header->name, name) == 0) {

        http_header_set_value(header, (char *) value);
        return;
    }

    // replace existing header
    while (header->next != NULL) {

        header = header->next;

        if (strcasecmp(header->name, name) == 0) {

            http_header_set_value(header, (char *) value);
            return;
        }
    }

    header->next = http_header_new();

    http_header_set_name(header->next, (char *) name);
    http_header_set_value(header->next, (char *) value);
}

void http_header_add(struct http_request *hreq, const char *name, const char *value) {

    struct http_header *header = hreq->headers;

    if (header == NULL) {

        hreq->headers = http_header_new();

        http_header_set_name(hreq->headers, (char *) name);
        http_header_set_value(hreq->headers, (char *) value);
        return;
    }

    // replace existing header
    while (header->next != NULL) {

        header = header->next;
    }

    header->next = http_header_new();

    http_header_set_name(header->next, (char *) name);
    http_header_set_value(header->next, (char *) value);
}

void http_header_unset(struct http_request *hreq, const char *name) {

    struct http_header *head = hreq->headers;
    struct http_header *tmp;

    while (head != NULL && strcasecmp(head->name, name) == 0) {

        tmp = head;
        head = hreq->headers = head->next;

        http_header_free(tmp);
    }

    while (head != NULL && head->next != NULL) {

        if (strcasecmp(head->next->name, name) == 0) {

            tmp = head->next;
            head->next = head->next->next;
            http_header_free(tmp);
            continue;
        }

        head = head->next;
    }
}

void http_header_set_name(struct http_header *header, char *name) {

    if (header->name != NULL) {

        free(header->name);
    }

    size_t len = strlen(name);
    header->name = malloc(len + 1);

    memcpy(header->name, name, len);
    header->name[len] = '\0';
}

void http_header_set_value(struct http_header *header, char *value) {

    if (header->value != NULL) {

        free(header->value);
    }

    size_t len = strlen(value);
    header->value = malloc(len + 1);

    memcpy(header->value, value, len);
    header->value[len] = '\0';
}

/**
 * result must be freed
 * @param header
 * @return
 */
//char *http_headers_print(const struct http_header *header) {
//
//    struct http_header *head = (struct http_header *) header;
//    size_t len = 0;
//    size_t pos = 0;
//
//    char *buff = (char *) malloc(sizeof(char));
//
//    while (head != NULL) {
//
//        len = snprintf(NULL, 0, "%s: %s\r\n", head->name, head->value);
//        buff = (char *) realloc(buff, pos + len + 1);
//
//        sprintf(&buff[pos], "%s: %s\r\n", head->name, head->value);
//
//        pos += len;
//        head = head->next;
//    }
//
//    buff[pos] = '\0';
//    return buff;
//}

struct http_header *http_header_parse(const char *http_headers, size_t *len) {

    struct http_header *_hd = NULL;
    struct http_header *_next = NULL;
    char *str = strstr(http_headers, "\r\n\r\n");
    size_t header_len = 0;

    if (str == NULL) {

        *len = header_len = strlen(http_headers);
    }
    else {

        *len = str - http_headers + 4;
        header_len = *len - 4;
    }

    char header[*len - 3];
    memcpy(header, http_headers, header_len);

    header[header_len] = '\0';

    const char *delim = "\r\n";
    char *token = strtok(header, delim);

    while (token != NULL) {

        char *split = strstr(token, ":");

        if (split != NULL) {

            if (_hd == NULL) {

                _hd = _next = http_header_new();
            } else {

                _next->next = http_header_new();
                _next = _next->next;
            }

            size_t _len = strlen(token) - strlen(split);
            char name[_len + 1];

            memset(name, 0, _len);
            memcpy(name, token, _len);
            name[_len] = '\0';

            http_header_set_name(_next, (char *) name);

            while (token[++_len] == ' ');

            size_t val_len = strlen(token) - _len + 1;

            char value[val_len + 1];

            memset(value, 0, val_len);
            memcpy(value, &token[_len], val_len);

            value[val_len + 1] = '\0';

            http_header_set_value(_next, (char *) value);
        }

        token = strtok(NULL, delim);
    }

    return _hd;
}

void http_header_free(struct http_header *header) {

    if (header != NULL) {

        if (header->next != NULL) {

            http_header_free(header->next);
        }

        if (header->name != NULL) {

            free(header->name);
        }

        if (header->value != NULL) {

            free(header->value);
        }

        free(header);
    }
}

#endif //HTTP_CLIENT_C_HTTP_CLIENT_HEADER_H

//
// Created by tbela on 2022-04-08.
//

#ifndef HTTP_CLIENT_C_HEADER_H
#define HTTP_CLIENT_C_HEADER_H

typedef struct http_header {

    char *name;
    char *value;
    struct http_header *next;
} http_header;

http_header *http_header_new();

http_header *http_header_clone(http_header *);

int http_header_exists(http_header *header, const char *name);

http_header *http_header_get(http_header *header, const char *name);

void http_header_set_name(http_header *header, char *name);

void http_header_set_value(http_header *header, char *value);

http_header *http_header_parse(const char *http_headers, size_t *len);

void http_header_free(http_header *header);

// def
http_header *http_header_new() {

    http_header *header = (http_header *) calloc(sizeof(http_header), 1);

    return header;
}

http_header *http_header_clone(http_header *headers) {

    if (headers == NULL) {

        return NULL;
    }

    http_header *header = http_header_new();
    http_header *next = header;
    http_header *current = headers;

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

int http_header_exists(http_header *header, const char *name) {

    http_header *head = header;

    while (head != NULL) {

        if (strcasecmp(head->name, name) == 0) {

            return 1;
        }

        head = head->next;
    }

    return 0;
}

http_header *http_header_get(http_header *header, const char *name) {

    http_header *head = header;
    http_header *result = NULL;
    http_header *tmp, *tmp2;

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

void http_header_set_name(http_header *header, char *name) {

    if (header->name != NULL) {

        free(header->name);
        header->name = NULL;
    }

    if(name != NULL) {

        header->name = strdup(name);
    }
}

void http_header_set_value(http_header *header, char *value) {

    if (header->value != NULL) {

        free(header->value);
        header->value = NULL;
    }

    if(value != NULL) {

        header->value = strdup(value);
    }
}

http_header *http_header_parse(const char *http_headers, size_t *len) {

    http_header *_hd = NULL;
    http_header *_next = NULL;
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

void http_header_free(http_header *header) {

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

#endif //HTTP_CLIENT_C_HEADER_H

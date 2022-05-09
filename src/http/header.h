//
// Created by tbela on 2022-04-08.
//

#ifndef HTTP_CLIENT_C_HEADER_H
#define HTTP_CLIENT_C_HEADER_H

#include <stdbool.h>
#include "stringx.h"

#define HTTP_HEADER_ATTR_TYPE_STRING 0
#define HTTP_HEADER_ATTR_TYPE_BOOL 1

typedef struct http_header_attribute {

    uint8_t type;
    char *name;
    char *value;

    struct http_header_attribute *next;
} http_header_attribute;

typedef struct http_header {

    char *name;
    char *value;
    http_header_attribute *attribute;
    struct http_header *next;
} http_header;

http_header *http_header_new();
http_header_attribute *http_header_attribute_new();

http_header *http_header_clone(http_header *);
http_header_attribute *http_header_attribute_clone(http_header_attribute *);

int http_header_exists(http_header *header, const char *name);

http_header *http_header_get(http_header *header, const char *name);
http_header_attribute *http_header_attribute_get(http_header *header, const char *name);

void http_header_set_name(http_header *header, char *name);

void http_header_set_value(http_header *header, char *value);

http_header *http_header_parse(const char *, size_t *);
http_header_attribute *http_header_attribute_parse(const char *);

char *http_header_print(http_header *);
char *http_header_attribute_print(http_header_attribute *);

void http_header_free(http_header *header);
void http_header_attribute_free(http_header_attribute *attr);

// def
http_header *http_header_new() {

    http_header *header = (http_header *) calloc(sizeof(http_header), 1);
    return header;
}

http_header_attribute *http_header_attribute_new() {

    http_header_attribute *attr = (http_header_attribute *) calloc(sizeof(http_header_attribute), 1);
    return attr;
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

http_header_attribute *http_header_attribute_clone(http_header_attribute *attr) {

    http_header_attribute *clone = http_header_attribute_new();

    clone->type = attr->type;
    clone->name = strdup(attr->name);

    if (attr->value != NULL) {

        clone->value = strdup(attr->value);
    }

    return clone;
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

http_header_attribute *http_header_attribute_get(http_header *header, const char *name) {

    http_header_attribute *head = header->attribute;

    while (head != NULL) {

        if (strcasecmp(head->name, name) == 0) {

            return http_header_attribute_clone(head);
        }

        head = head->next;
    }

    return NULL;
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

    if (value == NULL) {

        return;
    }

    char* context = NULL;
    char *search = strdup(value);
    char* token = strtok_r(search, ";", &context);

    if (header->attribute != NULL) {

        http_header_attribute_free(header->attribute);
        header->attribute = NULL;
    }

    if (strstr(token, "=") == NULL) {

        header->value = strdup(token);
        token = strtok_r(NULL, ";", &context);
    }

    if (token != NULL) {

        header->attribute = http_header_attribute_parse(&value[token - search]);
    }

    free(search);
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

//            memset(name, 0, _len);
            memcpy(name, token, _len);
            name[_len] = '\0';

            http_header_set_name(_next, (char *) name);

            while (token[++_len] == ' ');

            size_t val_len = strlen(token) - _len;

            char value[val_len + 1];

//            memset(value, 0, val_len - 1);
            memcpy(value, &token[_len], val_len);

            value[val_len] = '\0';
            http_header_set_value(_next, (char *) value);
        }

        token = strtok(NULL, delim);
    }

    return _hd;
}

http_header_attribute *http_header_attribute_parse(const char *http_headers) {


    http_header_attribute *attr = NULL;
    http_header_attribute *root = NULL;

    char* headers = strdup(http_headers);
    char* context = NULL;
    char* token = strtok_r(headers, ";", &context);

    if (token != NULL) {

        do {

            token = ltrim(token);

            if (attr == NULL) {

                attr = root = http_header_attribute_new();
            }

            else {

                attr->next = http_header_attribute_new();
                attr = attr->next;
            }

            char *attr_val = strstr(token, "=") ;

            if (attr_val == NULL) {

                attr->name = strdup(token);
                attr->type = HTTP_HEADER_ATTR_TYPE_BOOL;

                free(token);
                continue;
            }

            attr->type = HTTP_HEADER_ATTR_TYPE_STRING;

            size_t attr_len = attr_val - token;
            attr->name = malloc(attr_len + 1);

            memcpy(attr->name, token, attr_len);
            attr->name[attr_len] = '\0';
            attr->value = ltrim(&attr_val[1]);

            free(token);
        }

        while ((token = strtok_r(NULL, ";", &context)) != NULL);
    }

    free(headers);
    return root;
}

char *http_header_print(http_header *headers) {

    char *_h = (char *) malloc(1);
    size_t size, curr = 0;
    http_header *header = headers;

    _h[0] = '\0';

    while (header != NULL) {

        char *attr = header->attribute == NULL ? "" : http_header_attribute_print(header->attribute);
        char *sep = header->value == NULL || header->attribute == NULL ? "" : "; ";

        size = snprintf(NULL, 0, "%s: %s%s%s\r\n", header->name, header->value == NULL ? "" : header->value, sep, attr);

        _h = (char  *) realloc(_h, curr + size + 1);
        sprintf(&_h[curr], "%s: %s%s%s\r\n", header->name, header->value == NULL ? "" : header->value, sep, attr);
        curr += size;

        if (header->attribute != NULL) {

            free(attr);
        }

        header = header->next;
    }

    _h[curr] = '\0';
    return _h;
}

char *http_header_attribute_print(http_header_attribute *attribute) {

    char *_h = (char *) malloc(1);
    size_t size, curr = 0;
    http_header_attribute *attr = attribute;

    _h[0] = '\0';

    if (attr == NULL) {

        return _h;
    }

    if (attr->type == HTTP_HEADER_ATTR_TYPE_BOOL) {

        size = snprintf(NULL, 0, "%s", attr->name);

        _h = (char *) realloc(_h, curr + size + 1);
        sprintf(&_h[curr], " %s", attr->name);
        curr += size;
    }

    else {

        size = snprintf(NULL, 0, "%s=%s", attr->name, attr->value);
        _h = (char *) realloc(_h, curr + size + 1);
        sprintf(&_h[curr], "%s=%s", attr->name, attr->value);
        curr += size;
    }

    attr = attr->next;

    while (attr != NULL) {

        if (attr->type == HTTP_HEADER_ATTR_TYPE_BOOL) {

            size = snprintf(NULL, 0, "; %s", attr->name);
            _h = (char *) realloc(_h, curr + size + 1);
            sprintf(&_h[curr], "; %s", attr->name);
            curr += size;
        }

        else {

            size = snprintf(NULL, 0, "; %s=%s", attr->name, attr->value);
            _h = (char *) realloc(_h, curr + size + 1);
            sprintf(&_h[curr], "; %s=%s", attr->name, attr->value);
            curr += size;
        }

        attr = attr->next;
    }

    _h[curr] = '\0';
    return _h;
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

        if (header->attribute != NULL) {

            http_header_attribute_free(header->attribute);
            header->attribute = NULL;
        }

        free(header);
    }
}

void http_header_attribute_free(http_header_attribute *attr) {

    if (attr != NULL) {

        if (attr->type == HTTP_HEADER_ATTR_TYPE_STRING && attr->value != NULL) {

            free(attr->value);
        }

        if (attr->name != NULL) {

            free(attr->name);
        }

        if (attr->next != NULL) {

            http_header_attribute_free(attr->next);
        }

        free(attr);
    }
}

#endif //HTTP_CLIENT_C_HEADER_H

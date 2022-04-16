//
// Created by tbela on 2022-04-08.
//

#ifndef HTTP_CLIENT_C_HTTP_CLIENT_STRUCT_H
#define HTTP_CLIENT_C_HTTP_CLIENT_STRUCT_H

#include "stdbool.h"

#define HTTP_CLIENT_C_HTTP_MAX_REDIRECT 10;

typedef void (http_header_cb_ptr)(struct http_header *);
typedef void (http_response_body_cb_ptr)(const char*, size_t, struct http_header *, int);

typedef enum {
    HTTP_OPTION_URL,
    HTTP_OPTION_HEADER,
    HTTP_OPTION_BODY,
    HTTP_OPTION_METHOD,
    HTTP_OPTION_REQUEST_HEADER_CALLBACK,
    HTTP_OPTION_RESPONSE_HEADER_CALLBACK,
    HTTP_OPTION_RESPONSE_BODY_CALLBACK
} http_option;

typedef struct http_response http_response;

struct http_header {

    char *name;
    char *value;
    struct http_header *next;
};

struct http_request {
//    http_response *response;
    char *request_uri;
    int max_redirect;
    char *method;
    char *body;
    size_t body_len;
    struct http_header *headers;
    http_header_cb_ptr *request_header_cb;
    http_header_cb_ptr *response_header_cb;
    http_response_body_cb_ptr *response_body_cb;
};

/*
	Represents an HTTP html response
*/
struct http_response {
    char *redirect_ui;
//    char *body;
    char *redirect_uri;
    size_t body_len;
    bool redirected;
    int redirect_count;
    int status_code;
    char *status_text;
    struct http_header *headers;
};

#endif //HTTP_CLIENT_C_HTTP_CLIENT_STRUCT_H

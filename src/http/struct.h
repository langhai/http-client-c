//
// Created by tbela on 2022-04-08.
//

#ifndef HTTP_CLIENT_C_HTTP_STRUCT_H
#define HTTP_CLIENT_C_HTTP_STRUCT_H

#define BUF_READ 16384
#define HTTP_CLIENT_C_HTTP_MAX_REDIRECT 10;

#include "http/header.h"

typedef void (http_header_cb_ptr)(http_header *);
typedef void (http_response_body_cb_ptr)(const wchar_t*, size_t, http_header *);

typedef enum {
    HTTP_OPTION_URL,
    HTTP_OPTION_HEADER,
    HTTP_OPTION_BODY,
    HTTP_OPTION_METHOD,
    HTTP_OPTION_REQUEST_HEADER_CALLBACK,
    HTTP_OPTION_RESPONSE_HEADER_CALLBACK,
    HTTP_OPTION_RESPONSE_BODY_CALLBACK
} http_option;

typedef struct http_request {
    char *request_uri;
    int max_redirect;
    char *method;
    char *body;
    size_t body_len;
    http_header *headers;
    http_header_cb_ptr *request_header_cb;
    http_header_cb_ptr *response_header_cb;
    http_response_body_cb_ptr *response_body_cb;
} http_request;

/*
	Represents an HTTP html response
*/
typedef struct http_response {
//    char *body;
    char *redirect_uri;
    size_t body_len;
    uint8_t redirected;
    int redirect_count;
    int status_code;
    char *status_text;
    http_header *headers;
} http_response;

void http_response_free(http_response *hresp);

http_request *http_request_new() {

    http_request *hreq = (http_request *) calloc(sizeof(http_request), 1);

    if (hreq != NULL) {

        hreq->max_redirect = HTTP_CLIENT_C_HTTP_MAX_REDIRECT;
    }

    return hreq;
}

http_response *http_response_new() {

    http_response *hresp = (http_response *) calloc(sizeof(http_response), 1);

    return hresp;
}

/*
	Free memory of http_response
*/
void http_request_free(http_request *hreq) {

    if (hreq != NULL) {

        if (hreq->headers != NULL) {

            http_header_free(hreq->headers);
        }

        if (hreq->method != NULL) {

            free(hreq->method);
        }

        free(hreq);
    }
}

void http_response_free(http_response *hresp) {

    if (hresp != NULL) {


        if (hresp->status_text != NULL) {

            free(hresp->status_text);
        }
        if (hresp->headers != NULL) {

            http_header_free(hresp->headers);
        }

        free(hresp);
    }
}

#endif //HTTP_CLIENT_C_STRUCT_H

//
// Created by tbela on 2022-04-08.
//

#ifndef HTTP_CLIENT_C_HTTP_STRUCT_H
#define HTTP_CLIENT_C_HTTP_STRUCT_H

#define BUF_READ 8192
#define HTTP_CLIENT_C_HTTP_MAX_REDIRECT 50;

#include "http/header.h"

typedef struct http_request http_request;

typedef void (http_header_cb_ptr)(http_header *);
typedef void (http_response_body_cb_ptr)(const unsigned char*, size_t, http_header *);

typedef enum {
    HTTP_OPTION_URL,
    HTTP_OPTION_HEADER,
    HTTP_OPTION_BODY,
    HTTP_OPTION_METHOD,
    HTTP_OPTION_REQUEST_TIMEOUT,
    HTTP_OPTION_REQUEST_HEADER_CALLBACK,
    HTTP_OPTION_RESPONSE_HEADER_CALLBACK,
    HTTP_OPTION_RESPONSE_BODY_CALLBACK
} http_option;

typedef struct http_struct {

    http_header *headers;
    char *body;
    size_t body_len;
} http_struct;


typedef struct http_request {
    struct http_struct;
    char *request_uri;
    int max_redirect;
    char *method;
    http_header_cb_ptr *request_header_cb;
    http_header_cb_ptr *response_header_cb;
    http_response_body_cb_ptr *response_body_cb;
    struct timeval *request_timeout;
} http_request;

/*
	Represents an HTTP response
*/
typedef struct http_response {
    struct http_struct;
    char *redirect_uri;
    int redirect_count;
    int status_code;
    char *status_text;
} http_response;

void http_response_free(http_response *hresp);

http_request *http_request_new() {

    http_request *hreq = (http_request *) calloc(sizeof(http_request), 1);

    if (hreq != NULL) {

        hreq->max_redirect = HTTP_CLIENT_C_HTTP_MAX_REDIRECT;
        hreq->request_timeout = (struct timeval *) calloc(1, sizeof (struct timeval));
        hreq->request_timeout->tv_sec = 10;
        hreq->request_timeout->tv_usec = 0;
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

        if (hreq->request_uri != NULL) {

            free(hreq->request_uri);
        }

        if (hreq->body != NULL) {

            free(hreq->body);
        }

        if (hreq->method != NULL) {

            free(hreq->method);
        }

        free(hreq->request_timeout);
        free(hreq);
    }
}

void http_response_free(http_response *hresp) {

    if (hresp != NULL) {


        if (hresp->status_text != NULL) {

            free(hresp->status_text);
        }

        if (hresp->redirect_uri != NULL) {

            free(hresp->redirect_uri);
        }

        if (hresp->body != NULL) {

            free(hresp->body);
        }

        if (hresp->headers != NULL) {

            http_header_free(hresp->headers);
        }

        free(hresp);
    }
}

#endif //HTTP_CLIENT_C_STRUCT_H

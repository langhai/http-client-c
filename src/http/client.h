/*
	http-client-c
	Copyright (C) 2012-2013  Swen Kooij

	This file is part of http-client-c.

    http-client-c is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    http-client-c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with http-client-c. If not, see <http://www.gnu.org/licenses/>.

	Warning:
	This library does not tend to work that stable nor does it fully implement the
	standards described by IETF. For more information on the precise implementation of the
	Hyper Text Transfer Protocol:

	http://www.ietf.org/rfc/rfc2616.txt
*/

#ifndef HTTP_CLIENT_C_HTTP_CLIENT_H
#define HTTP_CLIENT_C_HTTP_CLIENT_H

//#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include "base64.h"
#include "url/parser.h"
#include "header.h"
#include "http/struct.h"
//#include "encoding/struct.h"
//#include "encoding/chunked.h"
#include "encoding/decode.h"
#include "error.h"
#include "stringx.h"
#include "struct.h"


#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#elif __linux__

#include <sys/socket.h>

#elif __FreeBSD__
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#error Platform not suppoted.
#endif


//typedef int (*http_header_cb_ptr)(const char*, int);

void http_request_header_set(http_request *hreq, const char *name, const char *value);

void http_request_header_add(http_request *hreq, const char *name, const char *value);

void http_request_header_unset(http_request *hreq, const char *name);

http_response *http_request_exec(http_request *hreq);

char *http_request_serialize(http_header *headers, const char *method, parsed_url *purl, char *body,
                             size_t body_len, size_t *len);

int http_request_send(http_response *hresp, http_header *request_headers, char *request, size_t request_len, char *host,
                      char *port, http_header_cb_ptr *request_header_cb,
                      http_header_cb_ptr *response_header_cb, http_response_body_cb_ptr *body_cb);

void http_request_option(http_request *hreq, http_option option, const void *val, size_t len);


void http_request_header_set(http_request *hreq, const char *name, const char *value) {

    http_header *header = hreq->headers;

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

void http_request_header_add(http_request *hreq, const char *name, const char *value) {

    http_header *header = hreq->headers;

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

void http_request_header_unset(http_request *hreq, const char *name) {

    http_header *head = hreq->headers;
    http_header *tmp;

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

/*
	Makes a HTTP request and returns the response
*/
http_response *http_request_exec(http_request *hreq) {

    const char *request_uri = hreq->request_uri;

    if (hreq->max_redirect == 0) {

        hreq->max_redirect = HTTP_CLIENT_C_HTTP_MAX_REDIRECT;
    }

    if (hreq->method == NULL) {

        hreq->method = strdup("GET");
    } else {

        str_to_upper(hreq->method, strlen(hreq->method));
    }

    if (request_uri == NULL) {

        return NULL;
    }

    parsed_url *purl = parse_url(hreq->request_uri);

    if (purl == NULL) {
        printf("Unable to parse url");
        return NULL;
    }

    if (hreq->body_len > 0) {

        char body_len[20];

        memset(body_len, 0, 20);
        sprintf(body_len, "%lu", hreq->body_len);

        http_request_header_set(hreq, "Content-Length", body_len);

        if (!http_header_exists(hreq->headers, "Content-Type")) {

            http_request_header_set(hreq, "Content-Type", "application/x-www-form-urlencoded");
        }
    }

    if (!http_header_exists(hreq->headers, "User-Agent")) {

        http_request_header_set(hreq, "User-Agent", "http-client-c");
    }

    http_request_header_set(hreq, "Connection", "close");

    http_response *hresp = http_response_new();

    if (hresp == NULL) {

        printf("Unable to allocate memory for HTTP response.");
        parsed_url_free(purl);
        return NULL;
    }

    size_t request_len = 0;
    http_header *headers = http_header_clone(hreq->headers);
    char *request = http_request_serialize(headers, hreq->method, purl, hreq->body, hreq->body_len, &request_len);

    http_header_free(headers);

    do {

        if (request_len < 0) {

            free(request);
            parsed_url_free(purl);
            return NULL;
        }

        printf("request:\n'%s'\n", request);

        http_header *request_headers = hreq->headers;

        int result = http_request_send(hresp, request_headers, request, request_len, purl->ip, purl->port,
                                       hreq->request_header_cb,
                                       hreq->response_header_cb, hreq->response_body_cb);

        free(request);

        if (result < 0) {

            parsed_url_free(purl);
            http_response_free(hresp);

            fprintf(stderr, "error: %s\n", http_client_error(result));
            return NULL;
        }

        if (strcasecmp(hreq->method, "OPTIONS") == 0 ||
            hresp->status_code == 304 ||
            hresp->status_code < 300 || hresp->status_code > 399 ||
            hreq->max_redirect == hresp->redirect_count) {

            break;
        }

//        break;
        hresp->redirect_count++;
        http_header *location = http_header_get(hresp->headers, "Location");

        purl = parse_url(location->value);
        http_header_free(hresp->headers);

//        free(request);

        // change HTTP method?
        const char *method = hresp->status_code == 307 ? hreq->method : (strcasecmp(hreq->method, "GET") == 0 ||
                                                                         strcasecmp(hreq->method, "HEAD") == 0
                                                                         ? hreq->method : "GET");

        // copy request headers
        headers = http_header_clone(hreq->headers);

        // cookies?



        if (strcasecmp(hreq->method, method) != 0) {

            fprintf(stderr, "switching HTTP method from %s to %s\n", hreq->method, method);

            if (hreq->body_len > 0 && hresp->redirect_count == 1) {

                fprintf(stderr, "ignoring %s payload\n", hreq->method);
            }

            request = http_request_serialize(headers, method, purl, NULL, 0, &request_len);
        } else {

            request = http_request_serialize(headers, method, purl, hreq->body, hreq->body_len, &request_len);
        }

        free(headers);


    } while (hreq->max_redirect > hresp->redirect_count);

    parsed_url_free(purl);

    /* Return response */
    return hresp;
}

char *http_request_serialize(http_header *headers, const char *method, parsed_url *purl, char *body,
                             size_t body_len, size_t *len) {

    if (purl->username != NULL) {

        /* Format username:password pair */
        size_t pwd_len = snprintf(NULL, 0, "%s:%s", purl->username, purl->password);
        char upwd[pwd_len]; //(char *) malloc(1024);
        memset(upwd, 0, pwd_len);
        sprintf(upwd, "%s:%s", purl->username, purl->password);

        /* Base64 encode */
        char *base64 = base64_encode((const unsigned char *) upwd, strlen(upwd));
        pwd_len = strlen(base64);
        char auth_header[pwd_len + 7]; // = realloc(base64, pwd_len + 7);

        sprintf(auth_header, "Basic %s", base64);

        http_header *auth = http_header_get(headers, "Authorization");

        if (auth == NULL) {

            auth = http_header_new();
            http_header_set_name(auth, "Authorization");

            http_header *header = headers;

            while (header->next != NULL) {

                header = header->next;
            }

            header->next = auth;
        }

        http_header_set_value(auth, auth_header);
        free(base64);
    }

    char *buff = calloc(1024 * sizeof(char), 1);
    size_t pos = strlen(buff);
    size_t buf_len;

    buf_len = snprintf(NULL, 0, "%s /%s%s%s HTTP/1.1\r\nHost:%s\r\n", method, purl->path == NULL ? "" : purl->path,
                       purl->query == NULL ? "" : "?", purl->query == NULL ? "" : purl->query, purl->host);

    sprintf(&buff[pos], "%s /%s%s%s HTTP/1.1\r\nHost:%s\r\n", method, purl->path == NULL ? "" : purl->path,
            purl->query == NULL ? "" : "?",
            purl->query == NULL ? "" : purl->query, purl->host);
    pos += buf_len;

    http_header *head = headers;

    while (head != NULL) {

        buf_len = snprintf(NULL, 0, "%s: %s\r\n", head->name, head->value);
        buff = (char *) realloc(buff, pos + buf_len + 1);

        sprintf(&buff[pos], "%s: %s\r\n", head->name, head->value);

        pos += buf_len;
        head = head->next;
    }

    buf_len = 2;
    buff = realloc(buff, pos + buf_len + 1);
    sprintf(&buff[pos], "\r\n");
    pos += buf_len;

    if (body_len > 0) {

        buf_len = body_len;
        buff = realloc(buff, pos + buf_len + 1);
        memcpy(&buff[pos], body, buf_len);
        pos += buf_len;
    }

    *len = pos;
    buff[pos] = '\0';

    return buff;
}

http_client_errors http_request_send(http_response *hresp, http_header *request_headers, char *request, size_t request_len, char *host,
                  char *port,
                  http_header_cb_ptr *request_header_cb, http_header_cb_ptr *response_header_cb,
                  http_response_body_cb_ptr *body_cb) {

    /* Declare variable */
    int sock;
    http_client_errors error_reason = HTTP_CLIENT_ERROR_OK;
    size_t tmpres;
    struct sockaddr_in *remote;

    /* Create TCP socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {

        error_reason = HTTP_CLIENT_ERROR_CONNECT;
        goto exit;
    }

    /* Set remote->sin_addr.s_addr */
    remote = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in *));
    remote->sin_family = AF_INET;
    tmpres = inet_pton(AF_INET, host, (void *) (&(remote->sin_addr.s_addr)));

    if (tmpres < 0) {

        error_reason = HTTP_CLIENT_ERROR_DNS;
        goto exit;
    } else if (tmpres == 0) {

        error_reason = HTTP_CLIENT_ERROR_HOST;
        goto exit;
    }

    remote->sin_port = htons(atoi((const char *) port));

    /* Connect */
    if (connect(sock, (struct sockaddr *) remote, sizeof(struct sockaddr)) < 0) {

        error_reason = HTTP_CLIENT_ERROR_CONNECT;
        goto exit;
    }

    if (request_header_cb != NULL) {

        request_header_cb(request_headers);
    }

    /* Send headers to server */
    size_t sent = 0;
    while (sent < request_len) {
        tmpres = send(sock, &request[sent], request_len - sent, 0);
        if (tmpres == -1) {

            error_reason = HTTP_CLIENT_ERROR_CONNECT;
            goto exit;
        }
        sent += tmpres;
    }

    /* Receive into response*/
//    char *response = (char *) malloc(0);
    char BUF[BUF_READ];
    http_transfer_encoding *te = NULL, *temp;
    ssize_t received_len = 0;
    size_t body_len = 0;
//    size_t response_len = 0;
//    uint8_t headers_parsed = 0;
    char *chunk = NULL;
    if ((received_len = recv(sock, BUF, BUF_READ - 1, 0)) > 0) {

        BUF[received_len] = '\0';
//        response_len += received_len;

//        if (!headers_parsed) {

        char *body_end = strstr(BUF, "\r\n\r\n");

        if (body_end != NULL) {

            char *first_line = strstr(BUF, "\r\n");

            if (first_line != NULL) {

                size_t status_len = first_line - BUF;
                char status_line[status_len];

                memcpy(status_line, BUF, status_len - 1);
                status_line[status_len] = '\0';

                char *status_text = strstr(status_line, " ");

                hresp->status_code = atoi(status_text);
                hresp->status_text = (char *) malloc(sizeof status_text);

                memcpy(hresp->status_text, &status_text[1], strlen(status_text));
            }

//                headers_parsed = 1;
            size_t headers_len = 0;
            hresp->headers = http_header_parse(BUF, &headers_len);

            if (response_header_cb != NULL) {

                response_header_cb(hresp->headers);
            }

            if (hresp->status_code > 299 && hresp->status_code < 400) {

                fprintf(stderr, "R%d ignoring response body\n", hresp->status_code);
                goto exit;
            }

            if (body_cb != NULL) {

                http_header *teh = http_header_get(hresp->headers, "Transfer-Encoding");

                char *data = &body_end[4];

                body_len = received_len - (body_end - BUF) - 4;

                if (teh != NULL) {

                    te = http_transfer_encoding_parse(teh->value);
                }

                error_reason = http_transfer_decode(te, sock, BUF, received_len, (body_end - BUF) + 4, hresp->headers, body_cb);

//                if (error_reason == HTTP_CLIENT_ERROR_TRANSFER_ENCODING) {

//                    fprintf(stderr, "unsupported transfer encoding: '%s'", te->value);
//                    error_reason = HTTP_CLIENT_ERROR_OK;
//                }

                goto exit;
            }
        }
//        }
    }

    if (received_len < 0) {

        error_reason = HTTP_CLIENT_ERROR_RECV;
        goto exit;
    }

    exit:

    if (remote != NULL) {

        free(remote);
    }

    if (chunk != NULL) {

        free(chunk);
    }

    if (te != NULL) {

        http_transfer_encoding_free(te);
    }

#ifdef _WIN32
    if (sock != -1) {

        closesocket(sock);
    }
#else
    if (sock != -1) {

        close(sock);
    }
#endif
    return error_reason;

}

void http_request_option(http_request *hreq, http_option option, const void *val, size_t len) {

    http_header *header;
    switch (option) {

        case HTTP_OPTION_URL:

            hreq->request_uri = (char *) val;
            break;

        case HTTP_OPTION_HEADER:

            header = (http_header *) val;
            http_request_header_set(hreq, header->name, header->value);
            break;

        case HTTP_OPTION_BODY:
            hreq->body = (char *) val;
            hreq->body_len = len == 0 ? strlen(hreq->body) : len;
            break;

        case HTTP_OPTION_REQUEST_HEADER_CALLBACK:
            hreq->request_header_cb = (http_header_cb_ptr *) val;
            break;

        case HTTP_OPTION_RESPONSE_HEADER_CALLBACK:
            hreq->response_header_cb = (http_header_cb_ptr *) val;
            break;

        case HTTP_OPTION_RESPONSE_BODY_CALLBACK:
            hreq->response_body_cb = (http_response_body_cb_ptr *) val;
            break;

        case HTTP_OPTION_METHOD:
            hreq->method = strdup(val);
            break;
    }
}

#endif
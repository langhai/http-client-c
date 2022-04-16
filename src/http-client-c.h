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

//#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
//#include <bits/mathcalls.h>
#include <errno.h>
#include <unistd.h>
#include "base64.h"
#include "urlparser.h"
#include "http-client-header.h"
#include "http-client-struct.h"

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

struct http_response *http_request_exec(struct http_request *hreq);

char *http_request_serialize(struct http_header *headers, const char *method, struct parsed_url *purl, char *body, size_t body_len, size_t *len);

int http_request_send(struct http_response *hresp, char *request, size_t request_len, char *host, char *port, http_header_cb_ptr *header_cb, http_response_body_cb_ptr *body_cb);

//struct http_response *http_get(char *url, char *custom_headers);
//
//struct http_response *http_head(char *url, char *custom_headers);
//
//struct http_response *http_post(char *url, char *custom_headers, char *post_data, size_t i);

struct http_request *http_request_new() {

    struct http_request *hreq = (struct http_request *) calloc(sizeof(struct http_request), 1);

    if (hreq == NULL) {

        return NULL;
    }

//    memset(hreq, 0, sizeof (struct http_request));

//    hreq->body_len = 0;
    hreq->max_redirect = HTTP_CLIENT_C_HTTP_MAX_REDIRECT;
    return hreq;
}

struct http_response *http_response_new() {

    struct http_response *hresp = (struct http_response *) calloc(sizeof(struct http_response), 1);

    if (hresp == NULL) {

        return NULL;
    }

//    memset(hresp, 0, sizeof (struct http_response));
//    hresp->body_len = 0;

    return hresp;
}

void http_request_option(struct http_request *hreq, http_option option, const void *val, int len);

void http_response_free(struct http_response *hresp);


/*
	Makes a HTTP request and returns the response
*/
struct http_response *http_request_exec(struct http_request *hreq) {

    const char *request_uri = hreq->request_uri;

    if (hreq->max_redirect == 0) {

        hreq->max_redirect = HTTP_CLIENT_C_HTTP_MAX_REDIRECT;
    }

    if (hreq->method == NULL) {

        hreq->method = "GET";
    }

    if (request_uri == NULL) {

        return NULL;
    }

    struct parsed_url *purl = parse_url(hreq->request_uri);

    if (purl == NULL) {
        printf("Unable to parse url");
        return NULL;
    }

    if (hreq->body_len > 0) {

        char body_len[20];

        memset(body_len, 0, 20);
        sprintf(body_len, "%lu", hreq->body_len);

        http_header_set(hreq, "Content-Length", body_len);

        if(!http_header_exists(hreq->headers, "Content-Type")) {

            http_header_set(hreq, "Content-Type", "application/x-www-form-urlencoded");
        }
    }

    if(!http_header_exists(hreq->headers, "User-Agent")) {

        http_header_set(hreq, "User-Agent", "http-client-c");
    }

    http_header_set(hreq, "Connection", "close");

    struct http_response *hresp = http_response_new();

    if (hresp == NULL) {

        printf("Unable to allocate memory for HTTP response.");
//        free(request);
        parsed_url_free(purl);
        return NULL;
    }

    size_t request_len = 0;
    struct http_header *headers = http_header_clone(hreq->headers);
    char *request = http_request_serialize(headers, hreq->method, purl, hreq->body, hreq->body_len, &request_len);

    free(headers);

    do {

        if (request_len < 0) {

            free(request);
            parsed_url_free(purl);
            return NULL;
        }

        printf("request:\n'%s'\n", request);

        int response_len = http_request_send(hresp, request, request_len, purl->ip, purl->port, hreq->response_header_cb, hreq->response_body_cb);

        if (response_len < 0) {

            free(request);
            parsed_url_free(purl);
            http_response_free(hresp);
            return NULL;
        }

        if (strcasecmp(hreq->method, "OPTIONS") == 0) {

            return hresp;
        }

        if (response_len < 0 || hresp->status_code < 300 || hresp->status_code > 399 || hreq->max_redirect == hresp->redirect_count) {
            break;
        }

//        break;
        hresp->redirect_count++;

        struct http_header *location = http_header_get(hresp->headers, "Location");

        purl = parse_url(location->value);
        http_header_free(hresp->headers);

        free(request);

        // change HTTP method?
        const char *method = hresp->status_code == 307 ? hreq->method : (strcasecmp(hreq->method, "GET") == 0 ||
                strcasecmp(hreq->method, "HEAD") == 0 ? hreq->method : "GET");

        // copy request headers
        headers = http_header_clone(hreq->headers);

        // cookies?



        if (strcasecmp(hreq->method, method) != 0) {

            fprintf(stderr, "switching HTTP method from %s to %s\n", hreq->method, method);

            if (hreq->body_len > 0 && hresp->redirect_count == 1) {

                fprintf(stderr, "ignoring %s payload\n", hreq->method);
            }

            request = http_request_serialize(headers, method, purl, NULL, 0, &request_len);
        }

        else {

            request = http_request_serialize(headers, method, purl, hreq->body, hreq->body_len, &request_len);
        }

        free(headers);


    } while (hreq->max_redirect > hresp->redirect_count);

    parsed_url_free(purl);

    /* Return response */
    return hresp;
}

char *http_request_serialize(struct http_header *headers, const char *method, struct parsed_url *purl, char *body, size_t body_len, size_t *len) {

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

        struct http_header *auth = http_header_get(headers, "Authorization");

        if (auth == NULL) {

            auth = http_header_new();
            http_header_set_name(auth, "Authorization");

            struct http_header *header = headers;

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

    size_t i = strlen(method);
    char _method[i + 1];

    _method[i] = '\0';

    while (i--) {

        _method[i] = (char) toupper((int) method[i]);
    }

    buf_len = snprintf(NULL, 0, "%s /%s%s%s HTTP/1.1\r\nHost:%s\r\n", _method, purl->path, purl->query == NULL ? "" : "?", purl->query == NULL ? "" : purl->query, purl->host);
//    printf("buff: %p\n", buff);

    sprintf(&buff[pos], "%s /%s%s%s HTTP/1.1\r\nHost:%s\r\n", _method, purl->path, purl->query == NULL ? "" : "?", purl->query == NULL ? "" : purl->query, purl->host);
    pos += buf_len;

    struct http_header *head = headers;

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

int http_request_send(struct http_response *hresp, char *request, size_t request_len, char *host, char *port, http_header_cb_ptr *header_cb, http_response_body_cb_ptr *body_cb) {


    /* Declare variable */
    int sock;
    size_t tmpres;
    struct sockaddr_in *remote;

    /* Create TCP socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {

        printf("Can't create TCP socket");
        return -1;
    }

    /* Set remote->sin_addr.s_addr */
    remote = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in *));
    remote->sin_family = AF_INET;
    tmpres = inet_pton(AF_INET, host, (void *) (&(remote->sin_addr.s_addr)));

    if (tmpres < 0) {

        printf("Can't set remote->sin_addr.s_addr");
        free(remote);
        return -1;
    }
    else if (tmpres == 0) {

        printf("Not a valid IP");
        free(remote);
        return -1;
    }

    remote->sin_port = htons(atoi((const char *) port));

    /* Connect */
    if (connect(sock, (struct sockaddr *) remote, sizeof(struct sockaddr)) < 0) {
        free(remote);
        printf("Could not connect");
        return -1;
    }

    /* Send headers to server */
    size_t sent = 0;
    while (sent < request_len) {
        tmpres = send(sock, &request[sent], request_len - sent, 0);
        if (tmpres == -1) {
            free(remote);
            printf("Can't send headers");
            return -1;
        }
        sent += tmpres;
    }

    /* Receive into response*/
//    char *response = (char *) malloc(0);
    char BUF[BUFSIZ];
    size_t received_len = 0;
    size_t body_len = 0;
    size_t response_len = 0;
    bool headers_parsed = 0;
    while ((received_len = recv(sock, BUF, BUFSIZ - 1, 0)) > 0) {

        BUF[received_len] = '\0';

//        printf("BUFF:\n'%s'\n", BUF);

//        response = (char *) realloc(response, response_len + received_len + 1);
//        memcpy(&response[response_len], BUF, received_len);
        response_len += received_len;
//        response[response_len] = '\0';

        if (!headers_parsed) {

            char *body_end = strstr(BUF, "\r\n\r\n");

            if (body_end != NULL)  {

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

                headers_parsed = true;
                size_t headers_len = 0;
                hresp->headers = http_header_parse(BUF, &headers_len);

                if (header_cb != NULL) {

                    header_cb(hresp->headers);
                }

                if (hresp->status_code > 299 && hresp->status_code < 400) {

                    fprintf(stderr, "R%d ignoring response body\n", hresp->status_code);

                    free(remote);

#ifdef _WIN32
                    closesocket(sock);
#else
                    close(sock);
#endif
                    return 0;
                }

                if (body_cb != NULL) {

//                    body_end += 4;
//                    printf("Body:\n'%s'\nBUF:\n'%s'\n", body_end, BUF);

                    body_len = received_len - (body_end - BUF) - 4;
                    body_cb(&body_end[4], body_len, hresp->headers, 0);
                }
            }
        }

        else {

            if (body_cb != NULL) {

                body_len += received_len;
                body_cb(BUF, received_len, hresp->headers, 0);
            }
        }
    }

    if (received_len < 0) {

        free(remote);
//        free(response);
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        fprintf(stderr, "Unable to receive");
        return -1;
    }

    if (body_cb != NULL) {

        body_cb(NULL, 0, (struct http_header *) hresp->headers, 1);
    }

    hresp->body_len = body_len;

    /* Reallocate response */
//	response = (char*)realloc(response, strlen(response) + 1);

    /* Close socket */
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif

    /* Parse status code and text */
//    char *status = get_until(response, "\r\n");
//    char *status_line = str_replace("HTTP/1.1 ", "", status);
//
//    free(status);
//
//    status = strndup(status_line, 4);
//    char *status_code = str_replace(" ", "", status);
//
//    free(status);
//
//    status = str_replace(status_code, "", status_line);
//    char *status_text = str_replace(" ", "", status);
//
//    free(status);
//    free(status_line);
//
//    hresp->status_code = status_code;
//    hresp->status_code = atoi(status_code);
//    hresp->status_text = status_text;


    /* Parse response headers */
//    char *headers = get_until(response, "\r\n\r\n");
//    hresp->response_headers = headers;

    /* Assign request headers */
//    hresp->request_headers = http_headers;

    /* Assign request url */
//    hresp->redirect_ui = purl;

    /* Parse body */
//    hresp->body_len = response_len - strlen(headers) - 4;
//	char *body = strstr(response, "\r\n\r\n");
//	body = str_replace("\r\n\r\n", "", body);
//    hresp->body = malloc(hresp->body_len + 1);
//    memcpy(hresp->body, &response[strlen(headers) + 4], hresp->body_len);

    free(remote);

    return response_len;
}


/*
	Free memory of http_response
*/
void http_request_free(struct http_request *hreq) {

    if (hreq != NULL) {

        if (hreq->headers != NULL) {

            http_header_free(hreq->headers);
        }

        free(hreq);
    }
}

void http_response_free(struct http_response *hresp) {

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

void http_request_option(struct http_request *hreq, http_option option, const void *val, int len) {

    struct http_header *header;

    switch (option) {

        case HTTP_OPTION_URL:

            hreq->request_uri = (char *) val;
            break;

        case HTTP_OPTION_HEADER:

            header = (struct http_header *) val;
            http_header_set(hreq, header->name, header->value);
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
            hreq->method = (char *) val;
            break;
    }
}

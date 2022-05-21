![My image](http://i.imgur.com/bYam0RK.png)

http-client-c
=============

A quick and dirty HTTP client library in and for C. The intention of http-client-c is to write an easy-to-use HTTP client in and for C.
Usage should be easy without hassle but still advanced and easy to control. http-client-c is almost fully comliant with the HTTP 1.1 standards.
http-client-c's code has been optimized to compile perfectly with all known C and C++ compilers. Altough the code is written
in C, it can be used in C++ code as well.

Basic Usage
===============

```c
    #include "http/client.h"

    http_request *request = http_request_new();

    http_request_option(request, HTTP_OPTION_URL, argv[1], 0);
    //  http_request_option(request, HTTP_OPTION_METHOD, "POST");
    //  http_request_option(request, HTTP_OPTION_BODY, "a=1&b=2");
    http_request_option(request, HTTP_OPTION_REQUEST_TIMEOUT, "5", 0);
    http_request_option(request, HTTP_OPTION_REQUEST_HEADER_CALLBACK, request_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_HEADER_CALLBACK, response_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_BODY_CALLBACK, response_body_cb, 0);
    // add an HTTP header, replace existing   
    http_request_header_set(request, "User-Agent", "Firefox 12");
    //  http_request_header_set(request, "Authorization", "Bearer <secret>");
    //  add an HTTP header, keep existing
    http_request_header_add(request, "Accept-Language", "en-US;q=0.6,en;q=0.4");

    // execute the request
    http_response *response = http_request_exec(request);

    http_request_free(request);
    http_response_free(response);
```

## http_request

the request structure.

```c

typedef struct http_request {
    http_header *headers;
    size_t body_len;
    char *body;
    char *request_uri;
    int max_redirect;
    char *method;
    http_header_cb_ptr *request_header_cb;
    http_header_cb_ptr *response_header_cb;
    http_response_body_cb_ptr *response_body_cb;
    struct timeval *request_timeout;
} http_request;

```

## create a request struct pointer.

```c
http_request *request = http_request_new();
```

## configure the request options

```c

// http_request *hreq - the request struct pointer
// http_option option - the option to configure
// const void *val - option payload
// size_t len - the payload length, only needed if the payload is a binary string, otherwise pass 0
// void http_request_option(http_request *hreq, http_option option, const void *val, size_t len)


    http_request_option(request, HTTP_OPTION_URL, argv[1], 0);
//    http_request_option(request, HTTP_OPTION_METHOD, "POST");
//    http_request_option(request, HTTP_OPTION_BODY, "a=1&b=2");
    http_request_option(request, HTTP_OPTION_REQUEST_TIMEOUT, "5", 0);
    http_request_option(request, HTTP_OPTION_REQUEST_HEADER_CALLBACK, request_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_HEADER_CALLBACK, response_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_BODY_CALLBACK, response_body_cb, 0);
    http_request_header_set(request, "User-Agent", "Firefox 12");
//    http_request_header_set(request, "Authorization", "Bearer <secret>");
    http_request_header_add(request, "Accept-Language", "en-US;q=0.6,en;q=0.4");
```

### http option flags
- HTTP_OPTION_URL: pass the url
- HTTP_OPTION_HEADER: pass an HTTP header as a _struct http_header_ pointer
- HTTP_OPTION_BODY: pass the request payload
- HTTP_OPTION_METHOD:  pass the HTTP method
- HTTP_OPTION_REQUEST_TIMEOUT: set the request timeout as a numeric string
- HTTP_OPTION_REQUEST_HEADER_CALLBACK: pass a request header callback (see ./test/main.c for an example)
- HTTP_OPTION_RESPONSE_HEADER_CALLBACK: pass a response header callback  (see ./test/main.c for an example)
- HTTP_OPTION_RESPONSE_BODY_CALLBACK: pass a response body callback  (see ./test/main.c for an example)

http_response
-------------
http_response is a structure that is returned by _http_request_exec_ method, it contains information about the response.
Please note that this function returns a pointer to an instance of http_response. The structure is as following:

```c

typedef struct http_response {
    http_header *headers;
    char *body;
    size_t body_len;
    char *redirect_uri;
    int redirect_count;
    int status_code;
    char *status_text;
} http_response;
```

##### *redirect_ui
The last redirected url.

##### *redirect_count
the HTTP redirect count

##### *body
The response body. always NULL if you set a response body callback

##### body_len
the response body length

##### status_code
the response HTTP status code

##### *status_text
This returns the text associated with the status code. For status code 200, OK will be returned.

##### *headers
response HTTP headers as _struct http_header_ pointer.

## HTTP response callback

Provide an HTTP response body callback to handle large HTTP response payload efficiently.
this will avoid allocating memory to hold the response.

## full example

```c

#include "http/client.h"

FILE *fp;

/**
 * response header callback
 * @param headers
 */
void response_header_cb(http_header *headers) {

    http_header *header = headers;

    fprintf(stderr, "headers received:\n");

    char *printed = http_header_print(headers);

    fprintf(stderr, "%s\r\n", printed);

    fwrite(printed, strlen(printed), 1, fp);
    fwrite("\r\n", 2, 1, fp);
    free(printed);
}

/**
 * request header callback
 * @param headers
 */
void request_header_cb(http_header *headers) {

    http_header *header = headers;

    fprintf(stderr, "headers sent:\n");

    char *printed = http_header_print(headers);

    fprintf(stderr, "%s\r\n", printed);

    fwrite(printed, strlen(printed), 1, fp);
    fwrite("\r\n", 2, 1, fp);
    free(printed);
}

/**
 * response body callback
 * @param chunk
 * @param chunk_len
 * @param headers
 */
void response_body_cb(const char *chunk, size_t chunk_len, http_header *headers) {

    if (chunk_len > 0) {

        fwrite(chunk, 1, chunk_len, fp);

        http_header *content_type = http_header_get(headers, "Content-Type");

        // if text content, dump to stderr
        if (content_type != NULL && strstr(content_type->value, "text/") != NULL) {

            fwrite(chunk, chunk_len, 1, stderr);
        }

        http_header_free(content_type);
    }
}

int main(int argc, char *argv[]) {

    if (argc <= 2) {

        fprintf(stderr, "Usage: \n$ %s URL DEST_FILE\n", argv[0]);
        exit(1);
    }

    char *filename = argc > 2 ? argv[2] : "";
    fprintf(stderr, "opening %s ...\n", filename);

    http_request *request = http_request_new();

    http_request_option(request, HTTP_OPTION_URL, argv[1], 0);
    //  http_request_option(request, HTTP_OPTION_METHOD, "POST");
    //  http_request_option(request, HTTP_OPTION_BODY, "a=1&b=2");
    http_request_option(request, HTTP_OPTION_REQUEST_TIMEOUT, "5", 0);
    http_request_option(request, HTTP_OPTION_REQUEST_HEADER_CALLBACK, request_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_HEADER_CALLBACK, response_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_BODY_CALLBACK, response_body_cb, 0);
    http_request_header_set(request, "User-Agent", "Firevox");
    //  http_request_header_set(request, "Authorization", "Bearer <secret>");
    http_request_header_add(request, "Accept-Language", "en-US;q=0.6,en;q=0.4");

    fp = fopen(filename, "wb");

    http_response *response = http_request_exec(request);

    fclose(fp);

    http_request_free(request);
    http_response_free(response);

    return 0;
}
```
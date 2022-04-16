//
// Created by tbela on 2022-04-07.
//
#include "http-client-c.h"

FILE *fp;

void respone_header_cb(struct http_header *headers) {

    struct http_header *header = headers;

    fprintf(stderr, "headers received:\n");

    while (header != NULL) {

        fprintf(stderr, "header -> '%s: %s'\n", header->name, header->value);
        header = header->next;
    }
}

void response_body_cb(const char *chunk, size_t chunk_len, struct http_header *headers, int stop) {

    if (chunk_len > 0) {

        fprintf(stderr, "writing %lu bytes of data:\n", chunk_len);
        fwrite(chunk, 1, chunk_len, fp);

        struct http_header *content_type = http_header_get(headers, "Content-Type");

        // if text content, dump to stderr
        if (content_type != NULL && strstr(content_type->value, "text/") != NULL) {

            fwrite(chunk, 1, chunk_len, stderr);
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc <= 2) {

        fprintf(fp, "Usage: \n$ %s URL DEST_FILE\n", argv[0]);
        exit(1);
    }

    char *filename = argc > 2 ? argv[2] : "";
    fprintf(stderr, "opening %s ...\n", filename);

    struct http_request *request = http_request_new();

//    printf("url: %s\n", argv[1]);

    http_request_option(request, HTTP_OPTION_URL, argv[1], 0);
//    http_request_option(request, HTTP_OPTION_METHOD, "POST", 0);
//    http_request_option(request, HTTP_OPTION_BODY, "a=1&b=2", 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_HEADER_CALLBACK, respone_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_BODY_CALLBACK, response_body_cb, 0);
//    http_header_set(request, "User-Agent", "IRON BOY");
//    http_header_set(request, "Authorization", "Bearer <secret>");
    http_header_add(request, "Authorization", "Bearer afeb5dv86");

    fp = fopen(filename, "wb");

    struct http_response *response = http_request_exec(request);

    fclose(fp);

    http_request_free(request);
    http_response_free(response);


//    struct http_response *response = http_get(argv[1], custom_headers);
//
//    if (response) {
//
//        fprintf(stderr, "request headers: [\n%s\n]\nresponse headers [\n%s\n]\n", response->request_headers,
//                response->response_headers);
//    }
//
//    if (response == NULL || response->status_code != 200) {
//
//        fprintf(stderr, "request failed with status code #%d\n", response->status_code);
//        http_response_free(response);
//        exit(1);
//    }
//
//    if (response->body_len > 0) {
//
//        fp = fopen(filename, "wb");
//
//        if (fp == NULL) {
//
//            fprintf(stderr, "cannot open file for writing: %s\n", filename);
//            http_response_free(response);
//            exit(1);
//        }
//
//        if (fwrite(response->body, 1, response->body_len, fp) != response->body_len) {
//
//            fprintf(stderr, "failed to write data into file: %s\n", filename);
//            http_response_free(response);
//            fclose(fp);
//            exit(1);
//        }
//
//        http_response_free(response);
//        fclose(fp);
//    }

    return 0;
}
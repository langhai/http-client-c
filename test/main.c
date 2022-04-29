//
// Created by tbela on 2022-04-07.
//
#include "http/client.h"

FILE *fp;

void response_header_cb(http_header *headers) {

    http_header *header = headers;

    fprintf(stderr, "headers received:\n");

    char *printed = http_header_print(headers);

    fprintf(stderr, "%s\r\n", printed);

    fwrite(printed, strlen(printed), 1, fp);
    fwrite("\r\n", 2, 1, fp);
    free(printed);
}

void request_header_cb(http_header *headers) {

    http_header *header = headers;

    fprintf(stderr, "headers sent:\n");

    char *printed = http_header_print(headers);

    fprintf(stderr, "%s\r\n", printed);

    fwrite(printed, strlen(printed), 1, fp);
    fwrite("\r\n", 2, 1, fp);
    free(printed);
}

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

//    size_t header_len = 0;
//    http_header *headers = http_header_parse("P3P: CP=\"This is not a P3P policy! See g.co/p3phelp for more info.\"\r\ncache-control: private, max-age=36500", &header_len);
//
//    char *print = http_header_print(headers);
//
//    fprintf(stderr, "%s", print);
//
//    free(print);
//    http_header_free(headers);
//
//    return 0;

    if (argc <= 2) {

        fprintf(fp, "Usage: \n$ %s URL DEST_FILE\n", argv[0]);
        exit(1);
    }

    char *filename = argc > 2 ? argv[2] : "";
    fprintf(stderr, "opening %s ...\n", filename);

    http_request *request = http_request_new();

//    printf("url: %s\n", argv[1]);

    http_request_option(request, HTTP_OPTION_URL, argv[1], 0);
//    http_request_option(request, HTTP_OPTION_METHOD, "POST");
//    http_request_option(request, HTTP_OPTION_BODY, "a=1&b=2");
    http_request_option(request, HTTP_OPTION_REQUEST_TIMEOUT, "5", 0);
    http_request_option(request, HTTP_OPTION_REQUEST_HEADER_CALLBACK, request_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_HEADER_CALLBACK, response_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_BODY_CALLBACK, response_body_cb, 0);
    http_request_header_set(request, "User-Agent", "Firevox");
//    http_request_header_set(request, "Authorization", "Bearer <secret>");
    http_request_header_add(request, "Accept-Language", "en-US;q=0.6,en;q=0.4");
    //
    //	fr-CA,en-CA;q=0.8,en-US;q=0.6,en;q=0.4,fr;q=0.2

    fp = fopen(filename, "wb");

    http_response *response = http_request_exec(request);

    fclose(fp);

    http_request_free(request);
    http_response_free(response);


//    http_response *response = http_get(argv[1], custom_headers);
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
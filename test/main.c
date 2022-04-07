//
// Created by tbela on 2022-04-07.
//
#include "http-client-c.h"

int main(int argc, char *argv[]) {

    if (argc <= 2) {

        fprintf(stderr, "Usage: \n$ %s URL DEST_FILE\n", argv[0]);
        exit(1);
    }

    char *filename = argc > 2 ? argv[2] : "";
    fprintf(stderr, "opening %s ...\n", filename);

    char *custom_headers = "User-Agent: Old-Brice\r\n";
    struct http_response *response = http_get(argv[1], custom_headers);

    if (response) {

        fprintf(stderr, "request headers: [\n%s\n]\nresponse headers [\n%s\n]\n", response->request_headers,
                response->response_headers);
    }

    if (response == NULL || response->status_code_int != 200) {

        fprintf(stderr, "request failed with status code #%d\n", response->status_code_int);
        http_response_free(response);
        exit(1);
    }

    if (response->body_len > 0) {

        FILE *fp = fopen(filename, "wb");

        if (fp == NULL) {

            fprintf(stderr, "cannot open file for writing: %s\n", filename);
            http_response_free(response);
            exit(1);
        }

        if (fwrite(response->body, 1, response->body_len, fp) != response->body_len) {

            fprintf(stderr, "failed to write data into file: %s\n", filename);
            http_response_free(response);
            fclose(fp);
            exit(1);
        }

        http_response_free(response);
        fclose(fp);
    }

    return 0;
}
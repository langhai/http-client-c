//
// Created by tbela on 2022-04-07.
//
#include "http/client.h"

FILE *fp = NULL;
char *charset = NULL;
char *filename = NULL;

/**
 * response header callback
 * @param headers
 */
void response_header_cb(http_header *headers) {

    if (fp == NULL) {

        // check content charset
//        http_header *header = http_header_get(headers, "Content-Type");
//
//        if (header != NULL) {
//
//            if (header->attribute != NULL) {
//
//                http_header_attribute *cs = http_header_attribute_get(header->attribute, "charset");
//
//                if (cs != NULL) {
//
//                    fprintf(stderr, "\n\ncharset = '%s'\n\n", cs->value);
//                    charset = strdup(cs->value);
//                }
//            }
//
//            http_header_free(header);
//            header = NULL;
//        }

        fp = fopen(filename, "wb");
    }

    fprintf(stderr, "headers received:\n");

    char *printed = http_header_print(headers);

    fprintf(stderr, "%s\r\n", printed);
    free(printed);
}

/**
 * request header callback
 * @param headers
 */
void request_header_cb(http_header *headers) {

    fprintf(stderr, "headers sent:\n");

    char *printed = http_header_print(headers);

    fprintf(stderr, "%s\r\n", printed);
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

    filename = argc > 2 ? argv[2] : "";
    fprintf(stderr, "opening %s ...\n", filename);

    http_request *request = http_request_new();

    http_request_option(request, HTTP_OPTION_URL, argv[1], 0);
//    http_request_option(request, HTTP_OPTION_METHOD, "POST");
//    http_request_option(request, HTTP_OPTION_BODY, "a=1&b=2");
    http_request_option(request, HTTP_OPTION_REQUEST_TIMEOUT, "5", 0);
    http_request_option(request, HTTP_OPTION_REQUEST_HEADER_CALLBACK, request_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_HEADER_CALLBACK, response_header_cb, 0);
    http_request_option(request, HTTP_OPTION_RESPONSE_BODY_CALLBACK, response_body_cb, 0);
    http_request_header_set(request, "User-Agent", "Bluefox");
//    http_request_header_set(request, "Authorization", "Bearer <secret>");
//    http_request_header_add(request, "Accept-Language", "zh-CN;q=0.9;en-US;q=0.6,en;q=0.4");

    http_response *response = http_request_exec(request);

    if (fp != NULL) {

        fclose(fp);
    }

    if (charset != NULL) {

        free(charset);
    }

    http_request_free(request);
    http_response_free(response);

    return 0;
}
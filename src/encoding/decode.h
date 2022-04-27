
#ifndef HTTP_CLIENT_C_ENCODING_DECODE_H
#define HTTP_CLIENT_C_ENCODING_DECODE_H

#include "encoding/struct.h"
#include "encoding/chunked.h"
#include "http/struct.h"

http_client_errors http_transfer_decode(http_transfer_encoding *te, int socket, const char *buf, size_t buf_len, size_t offset, http_header *headers,
                     http_response_body_cb_ptr *);

http_client_errors http_transfer_decode(http_transfer_encoding *te, int socket, const char *buf, size_t buf_len, size_t offset, http_header *headers,
                     http_response_body_cb_ptr *response_cb_ptr) {

    if (te != NULL) {

        if(strcmp(te->value, "chunked") == 0) {

            return http_chunked_transfer_decode(socket, buf, buf_len, offset, headers, response_cb_ptr);
        }

        return HTTP_CLIENT_ERROR_TRANSFER_ENCODING;
    }

    response_cb_ptr(&buf[offset], buf_len - offset, headers);

    size_t received_len = 0;
    char BUF[BUF_READ];

    while ((received_len = recv(socket, BUF, BUF_READ - 1, 0)) > 0) {

        response_cb_ptr(BUF, received_len, headers);
    }

    if (received_len < 0) {

        return HTTP_CLIENT_ERROR_DATA;
    }

    return 0;
}

#endif

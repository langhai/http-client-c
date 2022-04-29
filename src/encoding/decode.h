
#ifndef HTTP_CLIENT_C_ENCODING_DECODE_H
#define HTTP_CLIENT_C_ENCODING_DECODE_H

#include "encoding/struct.h"
#include "encoding/chunked.h"
#include "http/struct.h"

http_client_errors http_transfer_decode(http_transfer_encoding *te, int sock, const char *buf, size_t buf_len, size_t offset, http_request *, http_response *);

http_client_errors http_transfer_decode(http_transfer_encoding *te, int sock, const char *buf, size_t buf_len, size_t offset, http_request *hreq, http_response *hresp) {

    if (te != NULL) {

        if(strcmp(te->value, "chunked") == 0) {

            return http_chunked_transfer_decode(sock, buf, buf_len, offset, hreq, hresp);
        }

        return HTTP_CLIENT_ERROR_TRANSFER_ENCODING;
    }

    hreq->response_body_cb(&buf[offset], buf_len - offset, hresp->headers);

    size_t received_len = 0;
    char BUF[BUF_READ];

    while ((received_len = recv(sock, BUF, BUF_READ - 1, 0)) > 0) {

        hreq->response_body_cb(BUF, received_len, hresp->headers);
    }

    if (received_len < 0) {

        return HTTP_CLIENT_ERROR_DATA;
    }

    return 0;
}

#endif

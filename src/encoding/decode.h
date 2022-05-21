
#ifndef HTTP_CLIENT_C_ENCODING_DECODE_H
#define HTTP_CLIENT_C_ENCODING_DECODE_H

#include "encoding/struct.h"
#include "encoding/chunked.h"
#include "http/struct.h"

http_client_errors http_transfer_decode(http_transfer_encoding *te, int sock, char *buf, size_t buf_len, size_t offset, http_request *, http_response *);

http_client_errors http_transfer_decode(http_transfer_encoding *te, int sock, char *buf, size_t buf_len, size_t offset, http_request *hreq, http_response *hresp) {

    if (te != NULL) {

        if(strcmp(te->value, "chunked") == 0) {

            return http_chunked_transfer_decode(sock, buf, buf_len, offset, hreq, hresp);
        }

        return HTTP_CLIENT_ERROR_TRANSFER_ENCODING;
    }

    size_t body_len = 0;

    if (hreq->response_body_cb != NULL) {

        fprintf(stderr, "%s", &buf[offset]);
        hreq->response_body_cb(&buf[offset], buf_len - offset, hresp->headers);
    }
    else {

        body_len = buf_len - offset;
        hresp->body = malloc(body_len + 1);
        memcpy(hresp->body, &buf[offset], body_len);
    }

    size_t received_len = 0;

    while ((received_len = recv(sock, (void *) buf, BUF_READ - 1, 0)) > 0) {

        if (hreq->response_body_cb != NULL) {

            hreq->response_body_cb(buf, received_len, hresp->headers);
        }
        else {

            hresp->body = realloc(hresp->body, body_len + received_len + 1);
            memcpy(&hresp->body[body_len], buf, received_len);
            body_len += received_len;
        }
    }

    // TODO: if the content type is text - get wide_char_len instead of bytes_len
    if (hreq->response_body_cb == NULL) {

        hresp->body[body_len] = '\0';
        hresp->body_len = body_len;
    }

    if (received_len < 0) {

        return HTTP_CLIENT_ERROR_DATA;
    }

    return 0;
}

#endif

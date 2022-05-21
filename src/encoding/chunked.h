#ifndef HTTP_CLIENT_C_CHUNKED_H
#define HTTP_CLIENT_C_CHUNKED_H

#include "encoding/struct.h"
#include "error.h"
#include "stringx.h"
#include "charset/utf8.h"

int http_chunked_transfer_block_info(const char *buf, long buf_len, size_t *offset, size_t *data_len);

int http_chunked_transfer_decode(int sock, char *buf, size_t data_len, size_t offset, http_request *, http_response *);

void handle_response_body(const http_request *hreq, http_response *hresp, const unsigned char *mb_str, size_t _len);

/**
 *
 * @param buf buffer
 * @param buf_len buffer length
 * @param offset data offset
 * @return
 */
http_client_errors http_chunked_transfer_block_info(const char *buf, long buf_len, size_t *offset, size_t *data_len) {

    long pos = -1;

    *data_len = 0;
    *offset = 0;

    while (pos++ < buf_len) {

        if (buf[pos] == '\r' && pos + 1 < buf_len && buf[pos + 1] == '\n') {

            break;
        }
    }

    if (pos >= 1) {

        char chunk_size[pos + 1];

        memcpy(chunk_size, buf, pos);
        chunk_size[pos] = '\0';

        *offset = pos + 2;
        *data_len = hex2dec(chunk_size, pos);
    }

    return HTTP_CLIENT_ERROR_OK;
}

char *u8block_info(char *data, size_t bytes_read, size_t *mb_len) {

    char *block = (char *) calloc(bytes_read + 1, 1);

    u8strncpy(block, data, bytes_read);
    *mb_len = u8strlen(block);

    return block;
}

int http_chunked_transfer_decode(int sock, char *buf, size_t buf_len, size_t offset, http_request *hreq,
                                 http_response *hresp) {

    size_t block_size = 0;
    size_t block_offset = 0;

    // multibyte string length
    size_t mb_len;
    char *mb_str = NULL;
    ssize_t received_len = buf_len;

    int status;

    if (received_len - 1 > offset) {

        status = http_chunked_transfer_block_info(&buf[offset], buf_len - offset, &block_offset, &block_size);

        if (status < 0) {

            return HTTP_CLIENT_ERROR_RECV;
        }

        if (block_size == 0) {

            return HTTP_CLIENT_ERROR_OK;
        }

        while (block_size > 0) {

            partial_read:

            offset += block_offset;
            mb_str = u8block_info(&buf[offset], block_size, &mb_len);

            size_t _len = strlen(mb_str);

            if (_len == 0) {

                char *end = strstr(&buf[offset], "\r\n");
                
                if (end != NULL) {

                    block_size = end - &buf[offset];
                    handle_response_body(hreq, hresp, &buf[offset], block_size);
                    
                    offset += block_size;
                    goto block_info;
                }

                handle_response_body(hreq, hresp, &buf[offset], received_len - offset);
                return HTTP_CLIENT_ERROR_OK;
            }

            handle_response_body(hreq, hresp, mb_str, _len);

            hresp->body_len += _len;
            offset += _len;
            block_size -= mb_len;
            block_offset = 0;

            free(mb_str);
            mb_str = NULL;

            if (offset < received_len) {

                if (block_size > 0) {

                    goto partial_read;
                }

                if (block_size == 0) {

                    block_info:
                    offset += 2;
                    status = http_chunked_transfer_block_info(&buf[offset], received_len - offset, &block_offset,
                                                              &block_size);

                    if (status < 0) {

                        return HTTP_CLIENT_ERROR_RECV;
                    }

                    if (block_size == 0) {

                        return HTTP_CLIENT_ERROR_OK;
                    }

                    goto partial_read;
                }
            }

            received_len = recv(sock, (void *) buf, BUF_READ - 1, 0);
            offset = 0;

            if (received_len > 0) {

                buf[received_len] = '\0';

                goto partial_read;
            }

            if (received_len == 0) {

                return HTTP_CLIENT_ERROR_OK;
            }

            return HTTP_CLIENT_ERROR_RECV;
        }
    }

    return HTTP_CLIENT_ERROR_OK;
}

void handle_response_body(const http_request *hreq, http_response *hresp, const unsigned char *mb_str, size_t _len) {
    if (hreq->response_body_cb != NULL) {

        hreq->response_body_cb(mb_str, _len, hresp->headers);
    } else {

        if (hresp->body == NULL) {

            hresp->body = strdup(mb_str);
        } else {

            hresp->body = (char *) realloc(hresp->body, hresp->body_len + _len + 1);
            memcpy(&hresp->body[hresp->body_len], mb_str, _len);

            hresp->body[hresp->body_len + _len] = '\0';
        }
    }
}


#endif






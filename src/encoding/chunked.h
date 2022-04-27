#ifndef HTTP_CLIENT_C_CHUNKED_H
#define HTTP_CLIENT_C_CHUNKED_H

#include "encoding/struct.h"
#include "error.h"
#include "stringx.h"

int http_chunked_transfer_block_info(const char *buf, long buf_len, size_t *offset, size_t *data_len);

int http_chunked_transfer_decode(int socket, const char *buf, size_t buf_len, size_t offset, http_header *headers,
                                 http_response_body_cb_ptr *);

/**
 *
 * @param buf buffer
 * @param buf_len buffer length
 * @param offset data offset
 * @return
 */
http_client_errors http_chunked_transfer_block_info(const char *buf, long buf_len, size_t *offset, size_t *data_len) {

    long pos = -1;

    while (pos++ < buf_len) {

        if (buf[pos] == '\r' && pos + 1 < buf_len && buf[pos + 1] == '\n') {

            pos++;
            break;
        }
    }

    if (pos <= 1) {

        return HTTP_CLIENT_ERROR_OK;
    }

    char chunk_size[pos];

    memcpy(chunk_size, buf, pos - 1);

    chunk_size[pos] = '\0';

    *offset = pos + 1;
    *data_len = hex2dec(chunk_size, pos - 1);

    return HTTP_CLIENT_ERROR_OK;
}

int http_chunked_transfer_decode(int socket, const char *buf, size_t buf_len, size_t offset, http_header *headers,
                                 http_response_body_cb_ptr *body_cb_ptr) {

    size_t bytes_read = 0;
    size_t block_size = 0;
    size_t block_offset = 0;

    ssize_t received_len = buf_len;

    int status = 0;

    char data[BUF_READ];
    memcpy(data, buf, buf_len);

    while (received_len > 0) {

        block_info:
            status = http_chunked_transfer_block_info(&data[offset], buf_len - offset, &block_offset, &block_size);

            if (status < 0) {

                return HTTP_CLIENT_ERROR_DATA;
            }

            if (block_size == 0) {

                return 0;
            }

            while (block_size > 0) {

                if (block_size > buf_len - offset) {

                    bytes_read = buf_len - block_offset - offset;
                    body_cb_ptr(&data[offset] + block_offset, bytes_read, headers);

                    block_size -= bytes_read;

                    chunked_read:
                        received_len = recv(socket, data, BUF_READ - 1, 0);
                        offset = 0;

                        if (received_len < 0) {

                            return HTTP_CLIENT_ERROR_RECV;
                        }

                        if (received_len == 0) {

                            return HTTP_CLIENT_ERROR_OK;
                        }

                        buf_len = received_len;

                        if (received_len >= block_size) {

                            body_cb_ptr(&data[offset], block_size, headers);
                            offset = block_size + 2;
                            goto block_info;
                        } else {

                            body_cb_ptr(&data[offset], received_len, headers);
                            block_size -= received_len;
                            goto chunked_read;
                        }
                } else {

                    body_cb_ptr(&data[offset], block_size, headers);
                    offset += block_offset + block_size + 2;
                    goto block_info;
                }
            }
    }

    return HTTP_CLIENT_ERROR_OK;
}


#endif






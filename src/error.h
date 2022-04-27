

#ifndef HTTP_CLIENT_C_ERROR_H
#define HTTP_CLIENT_C_ERROR_H

typedef enum {

    HTTP_CLIENT_ERROR_OK = 0,
    HTTP_CLIENT_ERROR_CONNECT = -1,
    HTTP_CLIENT_ERROR_DNS = -2,
    HTTP_CLIENT_ERROR_HOST = -3,
    HTTP_CLIENT_ERROR_DATA = -4,
    HTTP_CLIENT_ERROR_RECV = -5,
    HTTP_CLIENT_ERROR_TRANSFER_ENCODING = -6,
    HTTP_CLIENT_PROTO = -7
} http_client_errors;

char *http_client_error(http_client_errors);

char *http_client_error(http_client_errors err) {

    switch (err) {

        case HTTP_CLIENT_ERROR_CONNECT:

            return "could not connect";

        case HTTP_CLIENT_ERROR_DNS:

            return "could not resolve dns";

        case HTTP_CLIENT_ERROR_HOST:

            return "invalid host";

        case HTTP_CLIENT_ERROR_DATA:

            return "unsupported encoding (content-encoding or transfer-encoding)";

        case HTTP_CLIENT_ERROR_RECV:

            return "error receiving data";

        case HTTP_CLIENT_ERROR_TRANSFER_ENCODING:

            return "unsupported transfer encoding";
    }

    return "";
}

#endif
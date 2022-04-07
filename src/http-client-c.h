/*
	http-client-c
	Copyright (C) 2012-2013  Swen Kooij

	This file is part of http-client-c.

    http-client-c is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    http-client-c is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with http-client-c. If not, see <http://www.gnu.org/licenses/>.

	Warning:
	This library does not tend to work that stable nor does it fully implement the
	standards described by IETF. For more information on the precise implementation of the
	Hyper Text Transfer Protocol:

	http://www.ietf.org/rfc/rfc2616.txt
*/

//#pragma GCC diagnostic ignored "-Wwrite-strings"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "base64.h"
#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <stdio.h>
	#pragma comment(lib, "Ws2_32.lib")
#elif __linux__
	#include <sys/socket.h>
#elif __FreeBSD__
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <arpa/inet.h>
#else
	#error Platform not suppoted.
#endif

#include <errno.h>
#include <unistd.h>
#include "stringx.h"
#include "urlparser.h"

/*
	Prototype functions
*/
struct http_response *http_req(char *http_headers, struct parsed_url *purl, unsigned long i);
struct http_response* http_get(char *url, char *custom_headers);
struct http_response* http_head(char *url, char *custom_headers);
struct http_response *http_post(char *url, char *custom_headers, char *post_data, size_t i);


/*
	Represents an HTTP html response
*/
struct http_response
{
	struct parsed_url *request_uri;
	char *body;
    size_t body_len;
	char *status_code;
	int status_code_int;
	char *status_text;
	char *request_headers;
	char *response_headers;
};

/*
	Handles redirect if needed for get requests
*/
struct http_response* handle_redirect_get(struct http_response* hresp, char* custom_headers)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = str_replace("Location: ", "", token);
				return http_get(location, custom_headers);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/*
	Handles redirect if needed for head requests
*/
struct http_response* handle_redirect_head(struct http_response* hresp, char* custom_headers)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = str_replace("Location: ", "", token);
				return http_head(location, custom_headers);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/*
	Handles redirect if needed for post requests
*/
struct http_response* handle_redirect_post(struct http_response* hresp, char* custom_headers, char *post_data, size_t len)
{
	if(hresp->status_code_int > 300 && hresp->status_code_int < 399)
	{
		char *token = strtok(hresp->response_headers, "\r\n");
		while(token != NULL)
		{
			if(str_contains(token, "Location:"))
			{
				/* Extract url */
				char *location = str_replace("Location: ", "", token);
				return http_post(location, custom_headers, post_data, len);
			}
			token = strtok(NULL, "\r\n");
		}
	}
	else
	{
		/* We're not dealing with a redirect, just return the same structure */
		return hresp;
	}
}

/*
	Makes a HTTP request and returns the response
*/
struct http_response* http_req(char *http_headers, struct parsed_url *purl, size_t header_length)
{
	/* Parse url */
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	int sock;
	int tmpres;
	char buf[BUFSIZ+1];
	struct sockaddr_in *remote;

	/* Allocate memeory for htmlcontent */
	struct http_response *hresp = (struct http_response*)malloc(sizeof(struct http_response));
	if(hresp == NULL)
	{
		printf("Unable to allocate memory for htmlcontent.");
		return NULL;
	}
	hresp->body = NULL;
    hresp->body_len = 0;
	hresp->request_headers = NULL;
	hresp->response_headers = NULL;
	hresp->status_code = NULL;
	hresp->status_text = NULL;

	/* Create TCP socket */
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
	    printf("Can't create TCP socket");
		return NULL;
	}

	/* Set remote->sin_addr.s_addr */
	remote = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in *));
	remote->sin_family = AF_INET;
  	tmpres = inet_pton(AF_INET, purl->ip, (void *)(&(remote->sin_addr.s_addr)));
  	if( tmpres < 0)
  	{
        free(remote);
    	printf("Can't set remote->sin_addr.s_addr");
    	return NULL;
  	}
	else if(tmpres == 0)
  	{
        free(remote);
		printf("Not a valid IP");
    	return NULL;
  	}

	remote->sin_port = htons(atoi(purl->port));

	/* Connect */
	if(connect(sock, (struct sockaddr *)remote, sizeof(struct sockaddr)) < 0)
	{
        free(remote);
	    printf("Could not connect");
		return NULL;
	}

	/* Send headers to server */
	int sent = 0;
	while(sent < header_length)
	{
	    tmpres = send(sock, &http_headers[sent], header_length-sent, 0);
		if(tmpres == -1)
		{
            free(remote);
			printf("Can't send headers");
			return NULL;
		}
		sent += tmpres;
	 }

	/* Recieve into response*/
	char *response = (char*)malloc(0);
	char BUF[BUFSIZ];
	size_t received_len = 0;
    size_t response_len = 0;
	while((received_len = recv(sock, BUF, BUFSIZ - 1, 0)) > 0)
	{
        BUF[received_len] = '\0';
		response = (char*)realloc(response, response_len + received_len + 1);
		memcpy(&response[response_len], BUF, received_len);
        response_len += received_len;
        response[response_len] = '\0';
    }

	if (received_len < 0)
    {
        free(remote);
		free(response);
        free(http_headers);
		#ifdef _WIN32
			closesocket(sock);
		#else
			close(sock);
		#endif
        fprintf(stderr,"Unable to receive");
		return NULL;
    }

	/* Reallocate response */
//	response = (char*)realloc(response, strlen(response) + 1);

	/* Close socket */
	#ifdef _WIN32
		closesocket(sock);
	#else
		close(sock);
	#endif

	/* Parse status code and text */
	char *status = get_until(response, "\r\n");
	char *status_line = str_replace("HTTP/1.1 ", "", status);

    free(status);

    status = strndup(status_line, 4);
	char *status_code = str_replace(" ", "", status);

    free(status);

    status = str_replace(status_code, "", status_line);
    char *status_text = str_replace(" ", "", status);

    free(status);
    free(status_line);

    hresp->status_code = status_code;
	hresp->status_code_int = atoi(status_code);
	hresp->status_text = status_text;


	/* Parse response headers */
	char *headers = get_until(response, "\r\n\r\n");
	hresp->response_headers = headers;

	/* Assign request headers */
	hresp->request_headers = http_headers;

	/* Assign request url */
	hresp->request_uri = purl;

	/* Parse body */
    hresp->body_len = response_len - strlen(headers) - 4;
//	char *body = strstr(response, "\r\n\r\n");
//	body = str_replace("\r\n\r\n", "", body);
	hresp->body = malloc(hresp->body_len + 1);
    memcpy(hresp->body, &response[strlen(headers) + 4], hresp->body_len);

    free(remote);
    free(response);

	/* Return response */
	return hresp;
}

/*
	Makes a HTTP GET request to the given url
*/
struct http_response* http_get(char *url, char *custom_headers)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);
    memset(http_headers, 0, 1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "GET /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "GET /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "GET /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "GET / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		char *upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		char *base64 = base64_encode(upwd, strlen(upwd));

		/* Form header */
		char *auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	/* Add custom headers, and close */
	if(custom_headers != NULL)
	{
		memcpy(&http_headers[strlen(http_headers)], "\r\n", 2);
        memcpy(&http_headers[strlen(http_headers)], custom_headers, strlen(custom_headers));
	}
	else
	{
        memcpy(&http_headers[strlen(http_headers)], "\r\n", 2);
	}

	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl, strlen(http_headers));

	/* Handle redirect */
	return handle_redirect_get(hresp, custom_headers);
}

/*
	Makes a HTTP POST request to the given url
*/
struct http_response* http_post(char *url, char *custom_headers, char *post_data, size_t len)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "POST /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->query, purl->host, len);
		}
		else
		{
			sprintf(http_headers, "POST /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->path, purl->host, len);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "POST /?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->query, purl->host, len);
		}
		else
		{
			sprintf(http_headers, "POST / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nContent-Length:%zu\r\nContent-Type:application/x-www-form-urlencoded\r\n", purl->host, len);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		char *upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		char *base64 = base64_encode(upwd, strlen(upwd));

		/* Form header */
		char *auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
	}

    size_t header_length = strlen(http_headers) + len;
    http_headers = (char*)realloc(http_headers,  header_length + 1);

    memcpy(&http_headers[strlen(http_headers)], post_data, len);

//	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl, header_length);

	/* Handle redirect */
	return handle_redirect_post(hresp, custom_headers, post_data, len);
}

/*
	Makes a HTTP HEAD request to the given url
*/
struct http_response* http_head(char *url, char *custom_headers)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "HEAD /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "HEAD /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "HEAD/?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "HEAD / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		char *upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		char *base64 = base64_encode(upwd, strlen(upwd));

		/* Form header */
		char *auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	if(custom_headers != NULL)
	{
		sprintf(http_headers, "%s%s\r\n", http_headers, custom_headers);
	}
	else
	{
		sprintf(http_headers, "%s\r\n", http_headers);
	}
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl, 0);

	/* Handle redirect */
	return handle_redirect_head(hresp, custom_headers);
}

/*
	Do HTTP OPTIONs requests
*/
struct http_response* http_options(char *url)
{
	/* Parse url */
	struct parsed_url *purl = parse_url(url);
	if(purl == NULL)
	{
		printf("Unable to parse url");
		return NULL;
	}

	/* Declare variable */
	char *http_headers = (char*)malloc(1024);

	/* Build query/headers */
	if(purl->path != NULL)
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "OPTIONS /%s?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "OPTIONS /%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->path, purl->host);
		}
	}
	else
	{
		if(purl->query != NULL)
		{
			sprintf(http_headers, "OPTIONS/?%s HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->query, purl->host);
		}
		else
		{
			sprintf(http_headers, "OPTIONS / HTTP/1.1\r\nHost:%s\r\nConnection:close\r\n", purl->host);
		}
	}

	/* Handle authorisation if needed */
	if(purl->username != NULL)
	{
		/* Format username:password pair */
		char *upwd = (char*)malloc(1024);
		sprintf(upwd, "%s:%s", purl->username, purl->password);
		upwd = (char*)realloc(upwd, strlen(upwd) + 1);

		/* Base64 encode */
		char *base64 = base64_encode(upwd, strlen(upwd));

		/* Form header */
		char *auth_header = (char*)malloc(1024);
		sprintf(auth_header, "Authorization: Basic %s\r\n", base64);
		auth_header = (char*)realloc(auth_header, strlen(auth_header) + 1);

		/* Add to header */
		http_headers = (char*)realloc(http_headers, strlen(http_headers) + strlen(auth_header) + 2);
		sprintf(http_headers, "%s%s", http_headers, auth_header);
	}

	/* Build headers */
	sprintf(http_headers, "%s\r\n", http_headers);
	http_headers = (char*)realloc(http_headers, strlen(http_headers) + 1);

	/* Make request and return response */
	struct http_response *hresp = http_req(http_headers, purl, 0);

	/* Handle redirect */
	return hresp;
}

/*
	Free memory of http_response
*/
void http_response_free(struct http_response *hresp)
{
	if(hresp != NULL)
	{
		if(hresp->request_uri != NULL) parsed_url_free(hresp->request_uri);
		if(hresp->body != NULL) free(hresp->body);
        if(hresp->status_code != NULL) free(hresp->status_code);
		if(hresp->status_text != NULL) free(hresp->status_text);
		if(hresp->request_headers != NULL) free(hresp->request_headers);
		if(hresp->response_headers != NULL) free(hresp->response_headers);
		free(hresp);
	}
}

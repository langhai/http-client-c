![My image](http://i.imgur.com/bYam0RK.png)

http-client-c
=============

A quick and dirty HTTP client library in and for C. The intention of http-client-c is to write an easy-to-use HTTP client in and for C.
Usage should be easy without hassle but still advanced and easy to control. http-client-c is almost fully comliant with the HTTP 1.1 standards.
http-client-c's code has been optimized to compile perfectly with all known C and C++ compilers. Altough the code is written
in C, it can be used in C++ code as well.

Basic Usage
===============
http_response 
-------------
http_response is a structure that is returned by all http_* methods, it contains information about the requse and the response.
Please note that all functions return a pointer to an insance of http_response. The structure is as following:

	struct http_response
	{
		struct parsed_url *request_uri;
		char *body;
		char *status_code;
		int status_code_int;
		char *status_text;
		char *request_headers;
		char *response_headers;
	};
	
#####*request_uri
This is an instance of the parsed_url structure, this contains the request URL and all information about the request
URL. Look up parsed_url for more information.

#####*body
This contains the response BODY (usually HTML).

#####*status_code
This contains the HTTP Status code returned by the server in plain text format.

#####status_code_int
This returns the same as status_code but as an integer.

#####*status_text
This returns the text associated with the status code. For status code 200, OK will be returned.

#####*request_headers
This contains the HTTP headers that were used to make the request.

#####*response_headers
Contains the HTTP headers returned by the server.

http_req()
-------------
http_req is the basis for all other http_* methodes and makes and HTTP request and returns an instance of the http_response structure.

The prototype for this function is:

	struct http_response* http_req(char *http_headers, struct parsed_url *purl)
	
A simple example is:
	
	struct parsed_url *purl = parse_url("http://www.google.com/");
	struct http_response *hrep = http_req("GET / HTTP/1.1\r\nHostname:www.google.com\r\nConnection:close\r\n\r\n", purl);

Please note that http_req does not handle redirects. (Status code 300-399)

http_get()
-------------
Makes an HTTP GET request to the specified URL. This function makes use of the http_req function. It specifies
the minimal headers required, in the second parameter you can specify extra headers.

The prototype for this function is:

	struct http_response* http_get(char *url, char *custom_headers)
	
A simple example is:

	struct http_response *hresp = http_get("http://www.google.com", "User-agent:MyUserAgent\r\n");
	
http_get does handle redirects automaticly. The basic headers used in this method:

	GET / HTTP/1.1
	Hostname:www.google.com
	Connection:close
	
http_post
------------
Makes an HTTP POST request to the specified URL. This function makes use of the http_req function. It specifies
the minimal headers required, in the second parameter you can specify extra headers. In the third parameter
the post data can be specified.

The prototype for this function is:

	struct http_response* http_post(char *url, char *custom_headers, char *post_data)
	
A simple example is:

	struct http_response *hresp = http_post("http://mywebsite.com/login.php", "User-agent:MyuserAgent\r\n", "username=Kirk&password=lol123");
	
http_post does handle redirects automaticly. The basic headers used in this method:

	POST /login.php HTTP/1.1
	Hostname:mywebsite.com
	Connection:close
	
	username=Kirk&password=lol123
	


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "http.h"

char* HTTP_parse_GET(char* request) {
	const char* TOKEN_1 = "GET http://";
	const char* TOKEN_2 = "/";
	const char* BAD_CHARS = " \\";

	int req_len = strlen(request);

	/* do work on local copy of request - side effect free */
	/* pointer from malloc, needed to free later */
	char* buffer = (char*)strdup(request);
	char* start = buffer; /* pointer into string */
	char* end;

	/* move to start of request string based on "GET http://" */
	/* would use strnstrn(), but libc on the SunOS boxes is ancient */
	start = strstr(start, TOKEN_1);
	if (start == NULL) {
		fprintf(stderr, "%s not found in request\n", TOKEN_1);
		goto parse_get_error;
	}
	start += strlen(TOKEN_1); /* advance start to end of first token */

	end = strstr(start, TOKEN_2); /* end should be delimited by "/" */
	if (end == NULL) {
		fprintf(stderr, "%s not found in request\n", TOKEN_2);
		goto parse_get_error;
	}
	*end = '\0'; // replace "/" with '\0'
	printf("Tokenised: %s\n", start);

	/* check token has no spaces or other nasties */
	if (strpbrk(start, BAD_CHARS)) {
		fprintf(stderr, "Bad characters found in hostname\n");
		goto parse_get_error;
	}

	/* return a freshly malloc()'d copy of start so it can be free()'d externally*/
	return (char*)strdup(start);

	parse_get_error:
		fprintf(stderr, "Request parsing failed.\n");
		free(buffer);
		return NULL;
}

char* HTTP_make_GET(char* hostname) {
	// maybe include X-Forwarded-For, but it's non-standard
	/* printf() family only counts /printed/ characters in length. string
	must end with printed character or \r\n bit gets cut off. seems hacky. */
	const char* format_str = "GET / HTTP/1.0\r\nHost: %s\r\n\r\n ";

	char dummy[1];
	/* there's no asprintf() on the Solaris boxes, so we'll do it ourselves */
	int len = snprintf(dummy, 1, format_str, hostname); // length of formatted string

	char* request = malloc(len + 1);
	if (request == NULL) {
		perror("Malloc failed to allocate space for GET request");
		goto make_get_error;
	}
	snprintf(request, len, format_str, hostname);
	return request;

	make_get_error:
		fprintf(stderr, "Failed to construct GET request.\n");
		free(request);
		return NULL;
}

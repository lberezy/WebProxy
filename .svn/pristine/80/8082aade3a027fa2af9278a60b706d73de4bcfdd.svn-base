#include <stdio.h>

#include "proxy.h"
#include "http.h"

#define REQUEST1 "GET http://www.goo gle.com/ HTTP/1.0\r\n\r\n"
#define REQUEST2 "GET http://towel.blinkenlights.nl/ HTTP/1.0\r\n\r\n"

int main(int argc, char const *argv[])
{
	char* result;

	printf("Testing request parsing...\nusing: %s", REQUEST1);
	result = HTTP_parse_GET(REQUEST1);
	if (result != NULL) {
		printf("Returned:\n%s\n", result);
	} else {
		printf("Returned NULL\n");
	}

	printf("Testing request parsing...\nusing: %s", REQUEST2);
	result = HTTP_parse_GET(REQUEST2);
	if (result != NULL) {
		printf("Returned:\n%s\n", result);
	} else {
		printf("Returned NULL\n");
	}


	printf("Testing request formation with 'google.com'\n");
	result = HTTP_make_GET("google.com");
	if (result != NULL) {
		printf("Returned:\n%s\n", result);
	} else {
		printf("Returned NULL\n");
	}

	printf("Testing request formation with ''\n");
	result = HTTP_make_GET(NULL);
	if (result != NULL) {
		printf("Returned:\n%s\n", result);
	} else {
		printf("Returned NULL\n");
	}
	return 0;
}

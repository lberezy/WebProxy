#ifndef _HTTP_H
#define _HTTP_H

/**
 * Attempts to parse a proxied HTTP GET request to return information about
 * the hostname.
 * A proxied HTTP GET request is of the form:
 * "GET http://hostname/ HTTP/1.0\r\n\r\n"
 *
 * If the parser decides the request string is malformed, a NULL pointer
 * will be returned.
 * @param  request 	C string containing the contents of HTTP GET request.
 * 
 * @return         	C string containing hostname portion on success
 *                  (must be free()'d), or NULL pointer on error.
 */

char* HTTP_parse_GET(char* request);

/**
 * Constructs a HTTP/1.0 GET request for the root (/) of a given hostname
 * @param  hostname: 	String of hostname to construct request around. 
 * 						(e.g. "google.com")
 * @return          	Returns a pointer to the constructed request on success
 *                      or NULL on error. (pointer must be free()'d after use)
 */
char* HTTP_make_GET(char* hostname);

#endif

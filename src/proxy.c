#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "http.h"
#include "proxy.h"
#include "log.h"


int init_server(char* port){
	struct addrinfo hints, *result, *result_list;
	int sock = -1;

	/* prepare information about desired socket */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // change to PF_UNSPEC for IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; // use TCP
	hints.ai_flags = AI_PASSIVE;	// use self address (0.0.0.0)


	if (getaddrinfo(NULL, port, &hints, &result_list) != 0) {
		goto error;
	}

	for (result = result_list; result != NULL; result = result->ai_next) {
		// attempt to craft a socket
		if ((sock = socket(result->ai_family, result->ai_socktype,
		result->ai_protocol)) < 0) {
			perror("Socket not suitable");
			continue;	// try next in list
		}
		// attempt to bind socket
		if (bind(sock, result->ai_addr, result->ai_addrlen) < 0) {
			close(sock);
			perror("Binding failed");
			continue;	// try next in list
		}
		break; /* if this point is reached, a suitable socket has been
				created and bound. */
	}

	if (sock < 0) { // exhausted getaddrinfo()'s list without success
		fprintf(stderr, "No valid sockets found to bind to!\n");
		goto error;
	}


	freeaddrinfo(result_list); // done with result list

	/* attempt to listen on socket */
	if (listen(sock, BACKLOG) < 0) {
		perror("Unable to listen with socket");
		goto error;
	}

	fprintf(stdout, "Server listening on port %s.\n", port);
	return sock;	// return listening socket

	error:
		fprintf(stderr, "Error initialising server.\n");
		return -1;
}


int read_request(int socket, char* buffer, int buffer_len) {
	memset(buffer, '\0', buffer_len); // empty buffer
	int recv_len = 0;
	int recv_bytes = 0;
	do {
		recv_len = recv(socket, buffer + recv_bytes,
			(buffer_len - 1) - recv_bytes, 0);
		recv_bytes += recv_len;
		printf("%s\n", buffer);
		// check buffer for end of header
		if (strstr(buffer, "\r\n\r\n") || strstr(buffer, "\n\n")) {
			return recv_bytes;
		}
    } while (recv_len);
    return recv_bytes;
}


int send_message(int socket, char* message, int msg_length) {
	int sent_bytes = 0;
	int ret_val;
	/* push request to remote host */
	int remaining_bytes = msg_length;
	while (remaining_bytes > 0 && errno != EPIPE) {
		ret_val = send(socket, message + sent_bytes, remaining_bytes, 0);

		if (ret_val >= 0) {
			sent_bytes += ret_val;
			remaining_bytes = msg_length - sent_bytes;
		} else {
			errno = EPIPE;
			perror("Pipe error");
		}
	}
	return sent_bytes;
}


int forward_bytes(int socket_1, int socket_2) {
	const int buffer_size = 4096;
	int forwarded_bytes = 0;
	int recv_bytes = 0;
	int ret_val;
	char buff[buffer_size];

	do {
		recv_bytes = recv(socket_1, &buff, buffer_size, 0);
		ret_val = send(socket_2, &buff, recv_bytes, 0);
		if (ret_val >= 0) {
			forwarded_bytes += ret_val;
		} else {
			errno = EPIPE;
			perror("Pipe error");
			fprintf(stderr, "Connection to client lost.\n");
		}

	} while (recv_bytes > 0 && errno != EPIPE);
	return forwarded_bytes;
}


void proxy_task(void* socket) {
#ifdef DEBUG
	printf("Proxy task initiated for socket_d %d\n", (int)request_sd);
	fflush(stdout);
#endif

	int remote_sd = -1;
	int request_sd = (int)socket;

	const static int buffer_len = 1024;
	/* allocate and clear buffer */
	char* recv_buffer = malloc(buffer_len + 1);
	memset(recv_buffer, '\0', buffer_len + 1);

	int recv_bytes = 0;
	int sent_bytes = 0;
	int recv_len = 0;

	char* req_hostname = NULL;
	char* constructed_req = NULL;
	int constructed_req_len = 0;

	/* read in request */
	recv_bytes += read_request(request_sd, recv_buffer, buffer_len + 1);

	req_hostname = HTTP_parse_GET(recv_buffer);
	if (req_hostname == NULL) {
		fprintf(stderr, "Unable to decode request.\n");
		goto error;
	}
	// clean the buffer
	memset(recv_buffer, '\0', buffer_len + 1);


	printf("Got hostname: %s\n", req_hostname);

	constructed_req = HTTP_make_GET(req_hostname);
	if (HTTP_make_GET == NULL) {
		fprintf(stderr, "Unable to construct request for forwarding.\n");
		goto error;
	}
	constructed_req_len = strlen(constructed_req);


	/* Connect to requested host */
	remote_sd = connect_to(req_hostname);
	if (remote_sd < 0) {
		fprintf(stderr, "Failed to connect to %s\n", req_hostname);
		goto error;
	}
	printf("Forwarding request to %s\n", req_hostname);
	printf("Req:\n%s", constructed_req);

	/* push request to remote host */
	sent_bytes += send_message(remote_sd, constructed_req, constructed_req_len);

	/* The other strategy to return the result to the client is to transfer
	individual bytes as they come in directly to the client with a single byte
	buffer - this would save memory and avoid buffer length problems,
	but have high syscall overhead */

	printf("Response from %s\n", req_hostname);
	/* forward response from remote host to requesting client */
	sent_bytes = 0;
	sent_bytes += forward_bytes(remote_sd, request_sd);
	printf("Forwarded %d bytes\n", sent_bytes);



	/* Write log entry */

	struct sockaddr_storage addr;
	socklen_t len = sizeof(addr);
	char client_ipstr[INET6_ADDRSTRLEN]; /* future proof and longer than IPv4 len */
	int port;
	char client_portstr[INET6_ADDRSTRLEN]; /* you never know... */
	char sent_bytes_str[INET6_ADDRSTRLEN];

	if (getpeername(request_sd, (struct sockaddr*)&addr, &len) < 0) {
		perror("getpeername");
		fprintf(stderr, "Failed to write log entry.\n");
	} else {
		/* we're IPv6 ready! */
		if (addr.ss_family == AF_INET6) {
			struct sockaddr_in6 *info = (struct sockaddr_in6 *)&addr;
			/* convert from network to presentation format */
			port = ntohs(info->sin6_port);
			inet_ntop(AF_INET6, &info->sin6_addr, client_ipstr, sizeof client_ipstr);
		} else {
			struct sockaddr_in *info = (struct sockaddr_in *)&addr;
			/* convert from network to presentation format */
			port = ntohs(info->sin_port);
			inet_ntop(AF_INET, &info->sin_addr, client_ipstr, sizeof client_ipstr);
		}

		/* it would probably be a good idea to modify write_log() to accept integers
		so that no int->str stuff is needed */
		snprintf(client_portstr, INET6_ADDRSTRLEN, "%d", port);
		snprintf(sent_bytes_str, INET6_ADDRSTRLEN, "%d", sent_bytes);

		write_log(client_ipstr, client_portstr, sent_bytes_str, req_hostname);
		//write_log("test","test2","test3","test4");
	}

	printf("Proxy task completed successfully!\n");

	close((int)request_sd);
	free(recv_buffer);
	free(req_hostname);

	return;

	error:
		fprintf(stderr, "Proxy task failed!\n");
		fflush(stderr);
		free(req_hostname);
		free(recv_buffer);
		close((int)request_sd);
		return;

}


int connect_to(char* hostname) {
	struct addrinfo hints, *result = NULL, *result_list = NULL;
	int sock = -1;

	/* information about desired socket */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; // change to PF_UNSPEC for IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(hostname, "80", &hints, &result_list) != 0) {
		fprintf(stderr, "gettaddrinfo() failed for given hostname\n");
		goto error;
	}

	for (result = result_list; result != NULL; result = result->ai_next) {
		/* try and make a socket */
		if ((sock = socket(result->ai_family, result->ai_socktype,
		result->ai_protocol)) < 0) {
			perror("Socket not suitable");
			/* try the next result */
			continue;
		}
		/* we have a usable socket, try to connect to it */
		if (connect(sock, result->ai_addr, result->ai_addrlen) < 0) {
			close(sock);
			perror("Unable to connect socket");
			continue;
		}
		/* A suitable socket has been created */
		break;
	}

	if (sock < 0) { // exhausted getaddrinfo()'s list without success
		fprintf(stderr, "No valid sockets found!\n");
		goto error;
	}

	freeaddrinfo(result_list); // done with result list
	return sock;	// return connected socket

	error:
		fprintf(stderr, "Could not connect to remote host.\n");
		close(sock);
		/* calling freeaddrinfo on a null pointer crashes on SunOS
		   because SunOS feeds on the screams and frustrations of developers. */
		if (result_list != NULL) {
			freeaddrinfo(result_list);
		}
		return -1;
}


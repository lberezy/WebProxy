#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "log.h"
#include "proxy.h"
#include "thread_pool.h"
#include "main.h"

#define THREAD_COUNT 3	// number of worker threads. set to num CPU cores - 1
#define QUEUE_SIZE 100	// pool queue size. max concurrent jobs


int listen_socket = -1;
int connection_fd = -1;
void proxy_shutdown(int signal);


/**
 * Shutdown routine. 	Cleans up server before closing.
 * @param signal 		not used.
 */
void proxy_shutdown(int signal) {
	printf("\nServer shutting down...\n");
	close(listen_socket);
	if (close_log()) {
		printf("Error closing log\n");
	} else {
		printf("Log closed\n");
	}
	printf("Done shutting down\n");
	exit(EXIT_SUCCESS);
}

int main(int argc, char const *argv[])
{
	const char* port = NULL;
	int port_num = -1;

	struct sockaddr remote_addr;
	socklen_t remote_addr_size = sizeof(remote_addr);

	threadpool_t *pool;

	signal(SIGPIPE, SIG_IGN);	/* supress SIGPIPE */
	signal(SIGINT, proxy_shutdown); /* catch ^C to gracefully shut down */


	/* check args */
	if (argc != 2) {
		printf("Usage: %s listen_port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	/* validate port */
	port = argv[1]; /* can be a number 0-65535 or a service name. e.g. "http"
					but check it's a valid port for this assignment only */
	port_num = atoi(port);
	if (port_num <= 0 || port_num > 65535) {
		fprintf(stderr, "Invalid port. Must be > 0 && < 65535\n");
		exit(EXIT_FAILURE);
	}

	/* initialise logging system */
	if (init_log(NULL, NULL) != 0) {
		fprintf(stderr, "Failed to initialise logging system.\n");
	}

	/* initialse thread pool */
	if ((pool = pool_create(THREAD_COUNT,QUEUE_SIZE)) == NULL) {
		fprintf(stderr, "Fatal error: Could not initialise thread pool!\n");
		exit(EXIT_FAILURE);
	}

	/* start listening server */
	if ((listen_socket = init_server((char*)port)) < 0) {
		fprintf(stderr, "Fatal error: Could not initialise server!\n");
		exit(EXIT_FAILURE);
	}

	printf("Proxy server ready\n");
	while(1) {
		connection_fd = accept(listen_socket, (struct sockaddr *)&remote_addr, &remote_addr_size);

		/* pass this connection to a worker thread to and handle */
		/* add task to pool */
		pool_add_task(pool, proxy_task, (void*)connection_fd);

	}
	/* unreachable */
	return EXIT_FAILURE;
}

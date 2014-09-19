#ifndef _PROXY_H
#define _PROXY_H


#define BACKLOG 16 /* controls rate at which socket can accept requests which
					works out to be BACKLOG / (avg. round trip time). Limited to
					SOMAXCONN (~128) on most Linux machines */

/**
 * Initialises a server socket listening on the specified port bound
 * to the first valid interface found.
 * @param  port Port/protocol to listen on as a C string.
 * @return      Returns -1 on error or the socket descriptor of
 *              the bound socket.
 */
int init_server(char* port);

/**
 * Reads from the client socket into the buffer until an end of header sequence
 * "\r\n\r\n" or ("\n\n" for netcat testing) is recieved. Empties buffer before
 * begining.
 * @param  socket		Client socket to read from.
 * @param  buffer     	Buffer to read into.
 * @param  buffer_len 	Length of buffer.
 * @return            	Number of bytes read from socket.
 */
int read_request(int socket, char* buffer, int buffer_len);

/**
 * Sends a message to a connected socket.
 * @param  socket     Socket to send to.
 * @param  message    Message to send.
 * @param  msg_length Length of message.
 * @return            Number of bytes sent.
 */
int send_message(int socket, char* message, int msg_length);

/**
 * Forwards bytes from socket 1 to socket 2 until socket 1 closes the connection.
 * @param  socket_1  Socket to accept bytes from.
 * @param  socket_2 Socket to send accepted bytes to.
 * @return          Number of bytes forwarded.
 */
int forward_bytes(int socket_1, int socket_2);

/**
 * Proxy task to be placed on thread pool job queue. Responds to an incoming 
 * request for a proxied HTTP GET request. Instantiates a connection to the
 * requested host and forward the returned message back to the original requester.
 * 
 * @param request_sd Socket descriptor of client that contacted the proxy server.
 */
void proxy_task(void* socket);

/**
 * Tries to make a connection to the desired hostname.
 * @param  hostname 	Hostname to try to connect to as C string.
 * @return          	Socket descriptor for connected socket or -1 on error.
 */
int connect_to(char* hostname);

#endif

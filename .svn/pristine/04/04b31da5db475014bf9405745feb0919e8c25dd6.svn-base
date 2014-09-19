#ifndef _LOG_H
#define _LOG_H

/**
 * @file log.h
 * @brief Logging system header file
 */

/**
* Initialise the logging system. Must be run before subsequent log calls.
 * @param  fname Filename for log location. NULL to use default ("proxy.log")
 * @param  mode  File mode. NULL to use default ("a")
 * @return  Return 0 on success, -1 on error.
 */
int init_log(char* fname, char* mode);

/**
 * Close the log file.
 * @return  Return 0 on success, -1 on error.
 */
int close_log(void);

/**
 * Writes a time-stamped entry to the system logfile. Log entries are of the form:
 * "date time,client ip address,port number,number bytes sent,requested hostname"
 * @param  client_ip    IP address of client as C string.
 * @param  port         Port number as C string.
 * @param  bytes_sent   Number of bytes sent as C string.
 * @param  req_hostname Requested hostname as C string.
 * @return              Return 0 on success, -1 on error.
 */
int write_log(char* client_ip, char* port, char* bytes_sent, char* req_hostname);

/**
 * Writes an arbitrary time-stamped string to system log file. Format:
 * "date time, custom string"
 * @param  format_str	Arbitrary format string.
 * @param  ...			Arguments for format string.
 * @return            	Return 0 on success, -1 on error.
 */
int write_log_str(const char* format_str, ...);

#endif

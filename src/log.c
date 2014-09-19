#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>

#include "log.h"

#define _LOG_FNAME_DEFAULT "proxy.log"
#define _LOG_MODE_DEFAULT "a"


static pthread_mutex_t log_lock;
static FILE* log_fd = NULL;


int init_log(char* fname, char* mode){
	int errnum;
	/* open log file in append mode */
	if (log_fd != NULL) {
		fprintf(stderr, "Error: Log system appears to be already initialised.\n");
		goto error;
	}
	if ((log_fd = fopen((fname == NULL) ? _LOG_FNAME_DEFAULT : fname,
	 					(mode == NULL) ? _LOG_MODE_DEFAULT : mode))
	 					 == NULL) {
		perror("Unable to open log file");
		goto error;
	}
	if ((errnum = pthread_mutex_init(&log_lock, NULL)) != 0) {
		perror("Could not init log mutex");
		goto error;
	}

	write_log_str("%s", "Log system initialised.");
	return 0;

	error:
		fprintf(stderr,"Unable to initialise log system!\n");
		fflush(stderr);
		return -1;
}

int close_log(void){
	if (log_fd == NULL) {
		fprintf(stderr, "Can't close log. Log system not initialised yet.\n");
		return -1;
	}
	pthread_mutex_lock(&log_lock);
	if (fclose(log_fd) != 0) {
		perror("Failed to close log file.\n");
		return -1;
	}
	log_fd = NULL;
	pthread_mutex_unlock(&log_lock);
	pthread_mutex_destroy(&log_lock);

	return 0;
}

int write_log(char* client_ip, char* port, char* bytes_sent, char* req_hostname) {
	char time_str[80];

	if (log_fd == NULL) {
		fprintf(stderr, "Can't write to log. Log system not initialised yet.\n");
		return -1;
	}
	pthread_mutex_lock(&log_lock);
	time_t now = time(NULL);
	strftime(time_str, 80, "%c", localtime((&now)));
	fprintf(log_fd, "%s,%s,%s,%s,%s\n", time_str,
		client_ip, port, bytes_sent, req_hostname);
	pthread_mutex_unlock(&log_lock);

	return 0;
}

int write_log_str(const char* format_str, ...) {
	char time_str[80];

	if (log_fd == NULL) {
		fprintf(stderr, "Can't write to log. Log system not initialised yet.\n");
		return -1;
	}
	pthread_mutex_lock(&log_lock);
	time_t now = time(NULL);
	strftime(time_str, 80, "%c", localtime((&now)));
	fprintf(log_fd, "%s, ", time_str); // write time
	/* write log line */
	va_list arglist;
	va_start(arglist, format_str);
	vfprintf(log_fd, format_str,arglist);
	va_end(arglist);
	fprintf(log_fd," \n");
	pthread_mutex_unlock(&log_lock);

	return 0;
}

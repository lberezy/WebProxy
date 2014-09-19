#include <stdio.h>

#include "log.h"

int main(int argc, char const *argv[])
{
	init_log("test.log", "w");
	write_log("192.168.1.1","8080","9000","google.com");
	close_log();
	write_log("192.168.1.1","8080","9000","google.com");
	return 0;
}

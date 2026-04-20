#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <syslog.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define PORT "9000" // the port client will be connecting to 
#define MAXDATASIZE 50000 // max number of bytes we can get at once
#define BUFFER_FILE "/var/tmp/aesdsocketdata" // the file used to write received content
#define BACKLOG 1000 //The number of pending connections allowed before refusing
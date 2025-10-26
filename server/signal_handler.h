#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H
#include <signal.h>
#include <sys/syslog.h>
#include <syslog.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>
#include <string.h>
#include <sys/syslog.h>

#include <pthread.h>





#define TWO 2U
#define ONE_BYTE 1U
#define ONE 1U
#define ZERO 0U
#define SUCCESS true
#define FAILURE false
#define BUFFER_SIZE 1024U
#define TEMP_FILE_PATH "/var/tmp/aesdsocketdata"





void signal_handler (const int signal_no);
bool setup_signal_handler();



#endif
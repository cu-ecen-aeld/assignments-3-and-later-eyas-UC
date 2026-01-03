#ifndef SOCKET_CORE_H
#define SOCKET_CORE_H

#include "queue.h"
#include <pthread.h>
#include <sys/socket.h>
#include "linkedlist.h"
void* socket_listen(void * arg);
void* connection_handler(void * thread_data);
void get_time(char * outstr);


#endif
#include <netinet/in.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <syslog.h>

#define END_CHAR '\n'
void* socket_listen(void * arg);
int main()
{


    printf("starting aesd socket server . . .\n");
    pthread_t t0;

    pthread_create(&t0, NULL, socket_listen,NULL);
    pthread_join(t0, NULL);

     
    exit(0);
}


void* socket_listen(void * arg)
{
int status;
    struct addrinfo hints;
    struct addrinfo *result;  // will point to the results
    
    // printf("sin address is %i",sa.sin_addr.s_addr);
    // printf("size of hints (addrinfo struct) is %zu\n", sizeof(hints));
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
    
    
    if((status = getaddrinfo("127.0.0.1","9000", &hints, &result)) !=0)
    {
        fprintf(stderr,"get address error: %s\n", gai_strerror(status));
        exit(1);
    }
    int socket_fd = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
    if (socket_fd == -1)
        {
        fprintf(stderr, "error in socket()\n");
    }


    int yes=1;
    //char yes='1'; // Solaris people use this

    // lose the pesky "Address already in use" error message
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    int bind_ret = bind(socket_fd, result->ai_addr, result->ai_addrlen);

    if (bind_ret != 0)
    {
        int listen_errno = errno;
        fprintf(stderr, "bind() failed: %s (errno=%d)\n", strerror(listen_errno), listen_errno);
    }

    // int connect_ret = connect(socket_fd, result->ai_addr, result->ai_addrlen);
    // if (connect_ret != 0)
    // {
    //     int listen_errno = errno;
    //     fprintf(stderr, "connect() failed: %s (errno=%d)\n", strerror(listen_errno), listen_errno);
    // }

    int listen_ret = listen(socket_fd, 10);
    if (listen_ret != 0) 
    {
        int listen_errno = errno;
        fprintf(stderr, "listen() failed: %s (errno=%d)\n", strerror(listen_errno), listen_errno);
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    printf("before listen\n");
    syslog(LOG_INFO, "before accepting new connection");


    int new_fd = accept(socket_fd,(struct sockaddr *)&their_addr, &addr_size);
    if (new_fd != 0) 
    {
        int listen_errno = errno;
        fprintf(stderr, "accept() failed: %s (errno=%d)\n", strerror(listen_errno), listen_errno);
    }
    else 
    {
        struct sockaddr * ca = (struct sockaddr *)&their_addr;
        syslog(LOG_INFO, "Accepted connection from %s ", ca->sa_data);
    }
    
    char buffer=' ';
    int ret_recv;
    char * to_write;
    uint size = 0;
    while ((ret_recv = recv(socket_fd, &buffer, 1, 0)) != 0 && (buffer != END_CHAR))
    {
        size ++;
        to_write = realloc(to_write, size);
    }
    if (0 != ret_recv)
    {
        int listen_errno = errno;
        fprintf(stderr, "recv() failed: %s (errno=%d)\n", strerror(listen_errno), listen_errno);
    }
    else
    {
        FILE * fd = fopen("/var/tmp/aesdsocketdata", "a");
    }
 

    // finally we need to free the server 
    close(socket_fd);
    freeaddrinfo(result);
}
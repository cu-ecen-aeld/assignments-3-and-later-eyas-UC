#include "signal_handler.h"
extern int signal_caught;
extern struct sigaction new_action;
extern char * to_write;
extern int socket_fd;
extern char ip4[INET_ADDRSTRLEN]; 
#define INIT_ALLOCATION BUFFER_SIZE
#define END_CHAR '\n'

void* socket_listen(void * arg)
{
int status;
    struct addrinfo hints;
    struct addrinfo *result;  // will point to the results
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if((status = getaddrinfo("0.0.0.0","9000", &hints, &result)) !=0)
    {
        fprintf(stderr,"get address error: %s\n", gai_strerror(status));
        exit(1);
    }
    socket_fd = socket(result->ai_family,result->ai_socktype,result->ai_protocol);
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
    // we can free result since we are not going to use it anymore
    freeaddrinfo(result);

    int listen_ret = listen(socket_fd, 100);
    if (listen_ret != 0) 
    {
        int listen_errno = errno;
        fprintf(stderr, "listen() failed: %s (errno=%d)\n", strerror(listen_errno), listen_errno);
    }

    struct sockaddr_storage their_addr;
    socklen_t addr_size = sizeof(their_addr);
    syslog(LOG_INFO, "before accepting new connection");
    
    int read_ret;
    char buffer[BUFFER_SIZE];
    uint size = INIT_ALLOCATION;
    size_t  current_count=0;
    to_write = malloc(size);
    if (to_write ==NULL)
    {
        int errno_malloc = errno;
        syslog(LOG_ERR, "error in malloc. errno is %s", strerror(errno_malloc));
    }

    while (true)
    {
        syslog(LOG_INFO, " ");
        syslog(LOG_INFO, "new data ------------------------------------------");
        syslog(LOG_INFO, " ");
        int new_fd = accept(socket_fd,(struct sockaddr *)&their_addr, &addr_size);
        if (new_fd <0) 
        {
            int listen_errno = errno;
            fprintf(stderr, "accept() failed: %s (errno=%d)\n", strerror(listen_errno), listen_errno);
        }
        else 
        {
            inet_ntop(AF_INET, &their_addr, ip4, INET_ADDRSTRLEN);
            printf("The IPv4 address is: %s\n", ip4);
            syslog(LOG_INFO, "Accepted connection from %s", ip4);
        }

        while((read_ret = read(new_fd, buffer, BUFFER_SIZE)) >0)
        {

            // size doubling section
            // **************************************************************//
            if (size <= current_count)
            {
                syslog(LOG_INFO,"increased size from %i",size);
                size = size * 2;
                to_write = realloc(to_write,(size) * sizeof(char));
                syslog(LOG_INFO,"to %i",size);
            }
            if (to_write == NULL)
            {
                syslog(LOG_ERR, "failed to reallocate memory");
            }
            memcpy(to_write + current_count , buffer,read_ret);
            current_count += read_ret; 
            
        }
        if (read_ret == 0)
        {
            if (to_write == NULL)
            {
                syslog(LOG_ERR, "failed to reallocate memory");
            }
            // syslog(LOG_INFO, "openning:<%s>",TEMP_FILE_PATH);
            int fd = open(TEMP_FILE_PATH, O_SYNC| O_RDWR  |O_CREAT | O_APPEND, S_IWUSR |S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
            // syslog(LOG_INFO, "writing:<%s> to <%s>-------- current_count = %i",to_write,TEMP_FILE_PATH, current_count);
            write(fd, to_write, current_count);
            // syslog(LOG_INFO, "syncing:<%s>",TEMP_FILE_PATH);
            fsync(fd);
            // syslog(LOG_INFO, "closing:<%s>",TEMP_FILE_PATH);
            // printf("to_write=\n<%s>",to_write);
            ssize_t  ret_send = send(new_fd,to_write,current_count,MSG_DONTWAIT);
            if (ret_send < 0 )
            {
                int read_errno = errno;
                syslog(LOG_ERR, "send() failed: %s (errno=%d)\n", strerror(read_errno), read_errno);
            }
            else
            {
                syslog(LOG_INFO, "send() passed with ret %li", ret_send);
            }
            close(fd);
            close(new_fd);
        }
    }
    syslog(LOG_ERR, "error in read: code = <%i>", read_ret);
   
    return 0;    
}
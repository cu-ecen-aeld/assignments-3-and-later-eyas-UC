
#include "signal_handler.h"
#include <sys/syslog.h>
#define END_CHAR '\n'

extern int signal_caught;
extern struct sigaction new_action;
extern char * to_write;
extern int socket_fd;
extern char ip4[INET_ADDRSTRLEN]; 


void* socket_listen(void * arg);
int main(int argc, char *argv[])
{
    if (setup_signal_handler() == FAILURE)
    {
        return -1;
    }
    int pid = 0;
    if (argc == TWO)
    {
        syslog(LOG_INFO, "the passed arguement is %s",argv[1]);
        if (strcmp(argv[1], "-d") == ZERO)
        {
            syslog(LOG_INFO, "launching in daemon mode. will fork the process");
            pid = fork();
            syslog(LOG_INFO, "PID is %i", pid);
        }
    }
    if (pid == ZERO)
    {
            printf("starting aesd socket server . . .\n");
            pthread_t t0;
            pthread_create(&t0, NULL, socket_listen,NULL);
            pthread_join(t0, NULL);
    }

    exit(0);
}


void* socket_listen(void * arg)
{
int status;
    struct addrinfo hints;
    struct addrinfo *result;  // will point to the results
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    if((status = getaddrinfo("localhost","9000", &hints, &result)) !=0)
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
    printf("before listen\n");
    syslog(LOG_INFO, "before accepting new connection");
    
    int read_ret;
    char buffer='x';
    to_write = malloc(1);
    uint size = 0;
    while (true)
    {
        syslog(LOG_INFO, "before accept");
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
        while((read_ret = read(new_fd, &buffer, ONE_BYTE)) >0)
        {
            to_write = realloc(to_write,(size +1 )* sizeof(char));
            if (to_write == NULL)
            {
                syslog(LOG_ERR, "failed to reallocate memory");
            }
            to_write[size] = buffer;
            size++;

            if (buffer == END_CHAR)
            {
                if (to_write == NULL)
                {
                    syslog(LOG_ERR, "failed to reallocate memory");
                }
                syslog(LOG_INFO, "openning:<%s>",TEMP_FILE_PATH);
                int fd = open(TEMP_FILE_PATH, O_SYNC| O_RDWR  |O_CREAT | O_APPEND, S_IWUSR |S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
                syslog(LOG_INFO, "writing:<%s> to %s",to_write,TEMP_FILE_PATH);
                write(fd, to_write, size);
                syslog(LOG_INFO, "syncing:<%s>",TEMP_FILE_PATH);
                fsync(fd);
                syslog(LOG_INFO, "closing:<%s>",TEMP_FILE_PATH);
                ssize_t  ret_send = send(new_fd,to_write,size,MSG_DONTWAIT);
                if (ret_send < 0 )
                {
                    int read_errno = errno;
                    syslog(LOG_ERR, "send() failed: %s (errno=%d)\n", strerror(read_errno), read_errno);
                }
                else
                {
                    syslog(LOG_INFO, "send() passed with ret %li", ret_send);
                }
                to_write = realloc(to_write,ZERO);
                close(fd);
                size = 0;
            }
        }
    }
    syslog(LOG_ERR, "error in read: code = <%i>", read_ret);
   
    return 0;    
}


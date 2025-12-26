#include "linkedlist.h"
#include "signal_handler.h"
// #include <sys/types.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/syslog.h>
#include <threads.h>
#include "socket_core.h"
#include <time.h>
#include <unistd.h>

extern int signal_caught;
extern struct sigaction new_action;
extern char * to_write;
extern int socket_fd;
extern char ip4[INET_ADDRSTRLEN];
extern ll * linked_list;
pthread_mutex_t timer_mutex;
pthread_mutex_t file_mutex;
pthread_mutex_t reply_mutex;
#define INIT_ALLOCATION BUFFER_SIZE
#define END_CHAR '\n'

void get_time(char * outstr)
{
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL)
    {
        perror("localtime");
    }

    if (strftime(outstr, 200, "timestamp:%a, %d %b %Y %T %z\n", tmp) == 0)
    {
        fprintf(stderr, "strftime returned 0");
    }
}



void * joining_thread_handler(void * passed_linkedlist)
{
    ll* linkedlist = (ll*) passed_linkedlist;
    while(1)
    {
        const struct timespec sleep_request= {.tv_nsec=1000};
        // struct timespec remainin= {.tv_nsec=1000};
        nanosleep(&sleep_request,NULL);
        sleep(1);
        pthread_mutex_lock(&linkedlist->mutex);
        node * it = linked_list->head;
        node * temp;
        pthread_mutex_unlock(&linkedlist->mutex);
        while(it != NULL)
        {
            pthread_mutex_lock(&linkedlist->mutex);
            if (it->data.completion == true)
            {
                pthread_join(it->data.thread_id,NULL);
                temp = it;
                it = it->next;
                remove_element_from_linked_list_no_mutex(linkedlist, temp->data.thread_id);
            }
            pthread_mutex_unlock(&linkedlist->mutex);

        }
    }
}
void* connection_handler(void *passed_fulldata)
{
    full_data_t* fulldata = (full_data_t*) passed_fulldata;
    pthread_t thread_id =  pthread_self();
    fulldata->thread_data.thread_id=thread_id;
    insert_element_to_linked_list(fulldata->linkedlist,fulldata->thread_data);
    node * thread_data = get_thread_data(fulldata->linkedlist,thread_id);
    if (thread_data == NULL)
    {
        printf("could not find thread id\n");
        return NULL;
    }

    if (thread_data->data.file_descriptor < 0) 
    {
        int listen_errno = errno;
        fprintf(stderr, "accept() failed: %s (errno=%d)\n", strerror(listen_errno), listen_errno);
    }
    else 
    {
        inet_ntop(AF_INET, &thread_data->data.their_addr, ip4, INET_ADDRSTRLEN);
        printf("The IPv4 address is: %s\n", ip4);
        syslog(LOG_INFO, "Accepted connection from %s", ip4);
    }
    set_thread_status(fulldata->linkedlist, thread_id, true);
    return 0;
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
    
    size_t read_ret;
    char buffer[BUFFER_SIZE]={0};
    size_t size = INIT_ALLOCATION;
    size_t  current_count=0;
    to_write = malloc(size);
    if (to_write ==NULL)
    {
        int errno_malloc = errno;
        syslog(LOG_ERR, "error in malloc. errno is %s", strerror(errno_malloc));
    }
    // create an empty linked list
    linked_list = malloc(sizeof(ll));
    init_linked_list(linked_list);
    pthread_t y;
    pthread_create(&y,NULL,joining_thread_handler,(void *)linked_list);
    while (true)
    {
        syslog(LOG_INFO, " ");
        syslog(LOG_INFO, "waiting to accept new connection ------------------------------------------");
        syslog(LOG_INFO, " ");
        int new_fd = accept(socket_fd,(struct sockaddr *)&their_addr, &addr_size);
        pthread_t thread_id;
        thread_data_t thread_data={new_fd,their_addr,thread_id, false};
        full_data_t full_set = {linked_list,thread_data};
        int thread_ret = pthread_create(&thread_id,NULL,connection_handler,(void *)&full_set);
        if (thread_ret != 0 )
        {
            int errno_local = errno;// errro occured
            syslog(LOG_ERR,"failed to create a thread\nerrno = %i,\n<%s>",errno_local,strerror(errno_local));
        }

        while((read_ret = read(new_fd, buffer, BUFFER_SIZE)) >0)
        {
            // size doubling section
            // **************************************************************//
            if (size <= (current_count + read_ret))
            {
                syslog(LOG_INFO,"increased size from %li",size);
                size = size * 2;
                to_write = realloc(to_write,size);
                syslog(LOG_INFO,"to %li",size);
            }
            if (to_write == NULL)
            {
                syslog(LOG_ERR, "failed to reallocate memory");
            }
            syslog(LOG_INFO, "received %s with length %li",buffer, read_ret);
            memcpy(to_write + current_count , buffer,read_ret);
            current_count += read_ret;
            if ( read_ret < BUFFER_SIZE)
            {
                syslog(LOG_INFO, "exiting loop size smaller than buffer (message ended)!");
                break;
                //exit this while loop
            }
            
        }
        if (to_write == NULL)
        {
            syslog(LOG_ERR, "failed to reallocate memory");
        }
        size_t  ret_send = send(new_fd,to_write,current_count,MSG_DONTWAIT);
        if (ret_send < 0 )
        {
            int read_errno = errno;
            syslog(LOG_ERR, "send() failed: %s (errno=%d)\n", strerror(read_errno), read_errno);
        }
        else
        {
            syslog(LOG_INFO, "send() passed with ret %li", ret_send);
        }
        close(new_fd);
        pthread_mutex_lock(&file_mutex);
        int fd = open(TEMP_FILE_PATH, O_SYNC| O_RDWR  |O_CREAT | O_APPEND, S_IWUSR |S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
        write(fd, to_write, current_count);
        fsync(fd);
        close(fd);
        pthread_mutex_unlock(&file_mutex);
        
    }
    syslog(LOG_ERR, "error in read: code = <%li>", read_ret);
   
    return 0;    
}

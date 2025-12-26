#ifndef LINKEDLIST
#define LINKEDLIST
#include <pthread.h>
#include <sys/socket.h>
#include <stdbool.h>


typedef struct _node node;
typedef struct __thread_data_t thread_data_t;
typedef struct linked_list ll;

typedef struct __thread_data_t
{
    int file_descriptor;
    struct sockaddr_storage their_addr;
    pthread_t thread_id;
    bool completion;
} thread_data_t;

typedef struct full_data
{
    ll * linkedlist;
    thread_data_t thread_data;
}full_data_t;
typedef struct _node
{
    struct _node * next;
    thread_data_t data;
} node;

typedef struct linked_list
{
    node * head;
    pthread_mutex_t mutex;
}ll;

void init_linked_list(ll * linked_list);
// creates a node and inserts data to it.
void insert_element_to_linked_list(ll *linked_list, thread_data_t data);
// removes an element from the linked list
void remove_element_from_linked_list(ll * linked_list, pthread_t thread_id);
void set_thread_status(ll * linked_list, pthread_t thread_id, bool status);
node * get_thread_data(ll * linked_list, pthread_t thread_id); //return an allocated data needs to be freed
// itarates and removes all elements of the linked list and then frees the linked list itself
void free_linked_list(ll * linkedlist);
void print_linked_list_thread_id(ll * linkedlist);
void print_node(node* n);
node * get_thread_data_by_fd(ll * linked_list, int fd);
void print_thread_data(thread_data_t data);
void remove_element_from_linked_list_no_mutex(ll * linked_list, pthread_t thread_id);


#endif
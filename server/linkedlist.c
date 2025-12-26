#include "linkedlist.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>


void init_linked_list(ll * linked_list)
{
    pthread_mutex_init(&linked_list->mutex,NULL);
    linked_list->head = NULL;
}
// creates a node and inserts data to it.
void insert_element_to_linked_list(ll *linked_list, thread_data_t data)
{

    // insert at the head.
    node * new_head = malloc(sizeof(node));
    new_head->data=data;
    node * old_head = linked_list->head;
    if (pthread_mutex_lock(&linked_list->mutex))
    {
        printf("error locking mutex");
    }
    linked_list->head = new_head;
    linked_list->head->next = old_head;
    if(pthread_mutex_unlock(&linked_list->mutex))
    {
        printf("error unlocking mutex");
    }
}
// removes an element from the linked list
void remove_element_from_linked_list(ll * linked_list, pthread_t thread_id)
{
    node * temp =linked_list->head;
    node * prev = temp;
    while (temp != NULL)
    {
        if (temp->data.thread_id == thread_id)
        {
            // in-place remove this element
            if (pthread_mutex_lock(&linked_list->mutex))
            {
                printf("error locking mutex");
            }        
            if (prev  == temp)
            {
                linked_list->head = temp->next;
            }
            else 
            {
                prev->next = temp->next;
            }
            free(temp);
            if(pthread_mutex_unlock(&linked_list->mutex))
            {
                printf("error unlocking mutex");
            }
            return;
        }
        else 
        {
            // itarate to the next element
            prev = temp;
            temp = temp->next;
        }
    }
}

void remove_element_from_linked_list_no_mutex(ll * linked_list, pthread_t thread_id)
{
    node * temp =linked_list->head;
    node * prev = temp;
    while (temp != NULL)
    {
        if (temp->data.thread_id == thread_id)
        {
            // in-place remove this element
            if (prev  == temp)
            {
                linked_list->head = temp->next;
            }
            else 
            {
                prev->next = temp->next;
            }
            free(temp);
            return;
        }
        else 
        {
            // itarate to the next element
            prev = temp;
            temp = temp->next;
        }
    }
}

void free_linked_list(ll * linkedlist)
{
    node * it = linkedlist->head;
    node * temp;
    while (it != NULL)
    {
        temp = it->next;
        free(it);
        it = temp;
    }
}

void print_linked_list_thread_id(ll * linkedlist)
{

    pthread_mutex_lock(&linkedlist->mutex);
    node * it = linkedlist->head;
    while (it != NULL)
    {
        printf("thread id is <%lu>, fd is <%i>\n",it->data.thread_id, it->data.file_descriptor);
        it = it->next;
    }
    pthread_mutex_unlock(&linkedlist->mutex);
}

void print_node(node* n)
{
    if (n ==NULL)
    {
        printf("NULL node!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        return;
    }
    printf("-------------------------------------------------------------------------------------------\n");
    printf("--------------------------------N O D E    I N F O R M A T I O N---------------------------\n");
    printf("status=<%s>,fd=<%i>, thread_id=<%li>\n",n->data.completion?"completed":"not finished",n->data.file_descriptor,n->data.thread_id);
    printf("-------------------------------------------------------------------------------------------\n");
}
void print_thread_data(thread_data_t data)
{
    printf("status=<%s>,fd=<%i>, thread_id=<%li>\n",data.completion?"completed":"not finished",data.file_descriptor, data.thread_id);
}
void set_thread_status(ll * linked_list, pthread_t thread_id, bool status)
{
    node * temp =linked_list->head;
    while (temp != NULL)
    {
        if (temp->data.thread_id == thread_id)
        {
            // in-place remove this element
            if (pthread_mutex_lock(&linked_list->mutex))
            {
                printf("error locking mutex");
            }
            temp->data.completion=status;
            if(pthread_mutex_unlock(&linked_list->mutex))
            {
                printf("error unlocking mutex");
            }
            return;
        }
        else 
        {
            // itarate to the next element
            temp = temp->next;
        }
    }
}


node * get_thread_data(ll * linked_list, pthread_t thread_id)
{
    node * temp =linked_list->head;
    while (temp != NULL)
    {
        if (temp->data.thread_id == thread_id)
        {
            // in-place remove this element
            node * return_data = malloc(sizeof(node));
            return_data->next = NULL;
            if (pthread_mutex_lock(&linked_list->mutex))
            {
                printf("error locking mutex");
            }
            return_data->data = temp->data;
            if(pthread_mutex_unlock(&linked_list->mutex))
            {
                printf("error unlocking mutex");
            }
            return  return_data;
        }
        else 
        {
            // itarate to the next element
            temp = temp->next;
        }
    }
    return NULL;
}


node * get_thread_data_by_fd(ll * linked_list, int fd)
{
    node * temp =linked_list->head;
    while (temp != NULL)
    {
        if (temp->data.file_descriptor == fd)
        {
            // in-place remove this element
            node * return_data = malloc(sizeof(node));
            return_data->next = NULL;
            if (pthread_mutex_lock(&linked_list->mutex))
            {
                printf("error locking mutex");
            }
            return_data->data = temp->data;
            if(pthread_mutex_unlock(&linked_list->mutex))
            {
                printf("error unlocking mutex");
            }
            return  return_data;
        }
        else 
        {
            // itarate to the next element
            temp = temp->next;
        }
    }
    return NULL;
}


/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes
 */
#include "aesd-circular-buffer.h"
#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#define AESD_DEBUG 1  //Remove comment on this line to enable debug

#undef PDEBUG             /* undef it, just in case */
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

struct aesd_dev
{
    /**
     * TODO: Add structure(s) and locks needed to complete assignment requirements
     */
    struct cdev cdev;     /* Char device structure      */
    struct mutex lock;
    /* here I should also add my circular buffer aka my data*/
    struct aesd_circular_buffer * c_buffer;
    struct aesd_buffer_entry partial_buffer;
//     /* not sure if quantum and quantum set are applicable here .... I think so in case reading or writing big data that needs to be handled */
//     int quantum;
//     int qset;
     // size will contain the size of the last element in the circular buffer
     unsigned int size;

};


#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */

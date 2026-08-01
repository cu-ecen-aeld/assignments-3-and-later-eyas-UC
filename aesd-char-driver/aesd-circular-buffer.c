/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */


#ifdef __KERNEL__
#include <linux/stddef.h>
#include <linux/string.h>
#else
#include <string.h>
#include <stdio.h>

#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    /**
    * TODO: implement per description
    */
    // easy case
    size_t accumulation = 0;

    // buffer->in_offs can never be greater than buffer->out_offs
    // the start is always from buffer->offs to buffer->in_offs
    // if in == out then buffer then check the full to loop through everything otherwise return null;
    if (buffer->full == true && (buffer->in_offs == buffer->out_offs))
    {
    }
    for (size_t i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++)
    {
        size_t current_element = (i +  buffer->in_offs) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
        // printf("char_offset=%zu out=%i in=%i i=%zu, current_element=%zu, accumulation=%zu, str=%s\n",char_offset, buffer->out_offs,buffer->in_offs, i, current_element,accumulation,buffer->entry[current_element].buffptr);
        if ((buffer->in_offs< buffer->out_offs) && (buffer->in_offs>current_element))
        {
            // printf("returning null here \n");
            return NULL;
        }
        size_t current_size = buffer->entry[current_element].size;
        if ((accumulation  + current_size) > char_offset)
        {
            // printf("returning current element here %li\n",current_element);
            * entry_offset_byte_rtn = char_offset - accumulation; 
            return &buffer->entry[current_element];
        }
        accumulation += current_size;


    }
    return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
struct aesd_buffer_entry * aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */
    // case where buffer is full, so we need to move the out_offs to the next entry to be overwritten
    struct aesd_buffer_entry * overwritten_entry = NULL;
    if (buffer->full)
    {
        // copy the old entry's contents out now: buffer->entry[buffer->in_offs] is about to be
        // overwritten in place below, so a pointer into the array would alias the new entry
o       verwritten_entry = &buffer->entry[buffer->in_offs];
        // this will essentially resets to 0 when LHS buffer->out_offs = 9 where
        // the RHS buffer->out_offs + 1 = 10 and 10 % 10 = 0;
        buffer->out_offs = (buffer->out_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    }
    // add the new entr]y to the buffer
    buffer->entry[buffer->in_offs] = *add_entry;
    // move the in_offs to the next location
    buffer->in_offs = (buffer->in_offs + 1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; 
    // if the buffer is now full, set the full flag to true 
    // the buffer has rotated and caught up to the out_offs
    if (buffer->in_offs == buffer->out_offs)
    {
        buffer->full = true;
    }
    return overwritten_entry;
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}

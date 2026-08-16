/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include "aesd-circular-buffer.h"
#include <linux/fs.h> // file_operations
#include "aesdchar.h"
#include "aesd_ioctl.h"

MODULE_DESCRIPTION("AESD character device driver");
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Eyas"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;
#define END_CHARACTER '\n'

int aesd_trim(struct aesd_dev * dev);
int aesd_trim(struct aesd_dev * dev)
{
    int i;
    for ( i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++)
    {
        if (dev->c_buffer.entry[i].buffptr != NULL)
        {
            kfree(dev->c_buffer.entry[i].buffptr);
            dev->c_buffer.entry[i].buffptr = NULL;
            dev->c_buffer.entry[i].size = 0;

        }
    }
    dev->c_buffer.full = false;
    dev->c_buffer.in_offs = 0;
    dev->c_buffer.out_offs = 0;
    return 0;
}


int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    /**
     * TODO: handle open
     */
	struct aesd_dev *dev; /* device information */
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;

    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */
    struct aesd_dev * dev = filp->private_data; // this is stored when open function is called
    // to read you need to lock the mutex first
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    size_t entry_offset = 0;
    struct aesd_buffer_entry * found_entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->c_buffer, *f_pos ,&entry_offset);
    if (found_entry == NULL)
    {
        // nothing to be read offset maybe too high
        mutex_unlock(&dev->lock);
        return 0;
    }
    // found the starting point for the data to be returned
    if (count + entry_offset >= found_entry->size)
    {
        count = found_entry->size - entry_offset;
    }
    // copy to user only the count - entry_offset
    size_t const remaining_bytes = copy_to_user(buf, found_entry->buffptr + entry_offset, count);
    if (remaining_bytes != 0)
    {
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }
    /* update file position after read is successful*/
    *f_pos += count;
    retval = count;

    mutex_unlock(&dev->lock);
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle write
     */
    // check last element for \n 
    // write command will be terminated with \n when it is done.
    // write operation which do not include \n char should be saved
    // and appended by future write operations
    // content for the most recent write commands should be saved.
    // memory associated with write commands more than 10 writes ago should be freed.
    //
    // count is the number of bytes 
    if (count == 0)
    {
        return 0;
    }
    struct aesd_dev * dev = filp->private_data;
    /* lock mutex*/
    if (mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;
    /* first allocate memeory with the given size "count" */
    char * allocated_memory = kmalloc(count, GFP_KERNEL);
    if (allocated_memory == NULL)
    {
        mutex_unlock(&dev->lock);
        printk(KERN_ERR "allocating memory went wrong!");
        return -ENOMEM;
    }
    


    size_t const not_copied_bytes = copy_from_user(allocated_memory, (const void *) buf, count);
    if (not_copied_bytes != 0)
    {
        kfree(allocated_memory);
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }
    PDEBUG("wrote %zu bytes: <%.*s>", count, (int)count, allocated_memory);
    size_t start = 0;
    size_t end = 0;
    for (size_t j = 0 ; j < count; j++)
    {
        if (allocated_memory[j] == END_CHARACTER)
        {
            end = j;
            if (dev->partial_buffer.size == 0U)
            {
                // create new entry and append it
                dev->partial_buffer.buffptr = kmalloc(end - start + 1, GFP_KERNEL);
                if (dev->partial_buffer.buffptr == NULL)
                {
                    kfree(allocated_memory);
                    mutex_unlock(&dev->lock);
                    return -ENOMEM;
                }
                dev->partial_buffer.size = end - start + 1;
                memcpy((void *)dev->partial_buffer.buffptr, (const void *)allocated_memory + start, end - start + 1);
            }
            else
            {
                // append the last parial data to the current (partial->size - 1) + j bytes
                size_t old_size = dev->partial_buffer.size;
                size_t new_size = old_size + end - start + 1;
                char * new_data = kmalloc(new_size, GFP_KERNEL);
                if (new_data == NULL)
                {
                    kfree(allocated_memory);
                    mutex_unlock(&dev->lock);
                    return -ENOMEM;
                }
                memcpy((void *)new_data, (const void *)dev->partial_buffer.buffptr, old_size);
                memcpy((void *)new_data + old_size, (const void *)allocated_memory + start, end - start + 1);
                kfree(dev->partial_buffer.buffptr);
                dev->partial_buffer.buffptr = new_data;
                dev->partial_buffer.size = new_size;
            }
            struct aesd_buffer_entry * old_entry;
            // here we will an entry in any case.
            if (NULL != (old_entry=aesd_circular_buffer_add_entry(&dev->c_buffer, &dev->partial_buffer)))
            {
                // need to free the overwritten entry
                kfree(old_entry->buffptr);
                old_entry->size = 0;
                old_entry->buffptr = NULL;
            }
            // since we have added an entry inside this if of this for loop then we need to reset the partial entry struct
            dev->partial_buffer.buffptr = NULL;
            dev->partial_buffer.size = 0U;
            start = end + 1;
        }
    }
    // handle last data
    if (start < count)
    {
        size_t remaining = count - start;
        // we have some data after the \n (END_CHARACTER)
        if (dev->partial_buffer.size == 0)
        {
            dev->partial_buffer.buffptr = kmalloc(remaining, GFP_KERNEL);
            if (!dev->partial_buffer.buffptr)
            {
                kfree(allocated_memory);
                mutex_unlock(&dev->lock);
                return -ENOMEM;
            }
            memcpy((void *)dev->partial_buffer.buffptr, (const void *)allocated_memory + start, remaining);
            dev->partial_buffer.size = remaining;

        }
        else
        {
            // This shouldn't normally happen if the loop processed all newlines,
            // but handle it for safety
            size_t new_size = dev->partial_buffer.size + remaining;
            char * new_data = kmalloc(new_size, GFP_KERNEL);
            if (!new_data)
            {
                kfree(allocated_memory);
                mutex_unlock(&dev->lock);
                return -ENOMEM;
            }
            memcpy((void *)new_data, (const void *)dev->partial_buffer.buffptr, dev->partial_buffer.size);
            memcpy((void *)new_data + dev->partial_buffer.size, (const void *)allocated_memory + start, remaining);
            kfree(dev->partial_buffer.buffptr);
            dev->partial_buffer.buffptr = new_data;
            dev->partial_buffer.size = new_size;
        }
    }
    kfree(allocated_memory);
    retval = count;
    mutex_unlock(&dev->lock);
    return retval;
}
static loff_t get_total_size(struct aesd_circular_buffer * buffer)
{
    loff_t size = 0;
    for (int i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++)
    {
            size = size + buffer->entry[i].size;
    }
    return size;
}

loff_t aesd_lseek(struct file * filp, loff_t off, int whence)
{
    loff_t new_position;
    struct aesd_dev *dev = filp->private_data;
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;
    loff_t total_size = get_total_size(&dev->c_buffer);
    mutex_unlock(&dev->lock);

    new_position = fixed_size_llseek(filp, off, whence, total_size);
    return new_position;
}

long int adjust_aesd_file_offset(struct file *filp, uint32_t write_cmd, uint32_t write_cmd_offset)
{
    /* range check */
    struct aesd_dev * dev = filp->private_data;
    struct aesd_circular_buffer * buffer = &dev->c_buffer;
    // last_index = buffer->in_offs;

    if (mutex_lock_interruptible(&dev->lock))
    {
        return -ERESTARTSYS;
    }
    if (write_cmd >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
    {
        printk(KERN_ERR "writing value greater than max supported ioctl seekto operation\n");
        mutex_unlock(&dev->lock);
        return -EINVAL;
    }
    /* finding the right size*/
    long int new_pos = 0;
    int target_index = (write_cmd + buffer->out_offs) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
    for (uint32_t i = 0; i < write_cmd; i++)
    {
        uint32_t index = (i + buffer->out_offs) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
        if ((buffer->entry[index].buffptr == NULL) && (buffer->entry[index].size == 0))
        {
            mutex_unlock(&dev->lock);
            return -EINVAL;
        }
        else
        {
            new_pos += buffer->entry[index].size;
        }
    }
    /* greater than or equal because if equal then you will be starting from after the last element */
    /* adding empty target index offset check*/
    if ((buffer->entry[target_index].buffptr == NULL ) || (write_cmd_offset >= buffer->entry[target_index].size))
    {
        printk(KERN_ERR "offset is bigger than entry size");
        mutex_unlock(&dev->lock);
        return -EINVAL;
    }
    else
    {
        new_pos += write_cmd_offset;
    }
    filp->f_pos = new_pos;
    mutex_unlock(&dev->lock);
    return new_pos;
}

long aesd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct aesd_dev * dev = filp->private_data;

    long int retval = 0;
    switch (cmd)
    {
        case AESDCHAR_IOCSEEKTO:
        {

            // some manual seeking here
            /* get the command from the user space */
            struct aesd_seekto command;
            if (copy_from_user(&command, (struct aesd_seekto __user *)arg, sizeof(command)))
            {
                retval = -EFAULT;
            }
            else
            {
               retval = adjust_aesd_file_offset(filp, command.write_cmd, command.write_cmd_offset);
            }
            break;
        }

        default:  /* redundant, as cmd was checked against MAXNR */
		    return -ENOTTY;
    }

    
    return retval;
}
// file ops goes under struct file
struct file_operations aesd_fops =
{
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
    .llseek = aesd_lseek,
    .unlocked_ioctl = aesd_ioctl,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }

    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */

    /* assuming that I have only one driver and not 4 like in the scull example. I will start by */
    /* setup the mutex by calling mutex init*/
    mutex_init(&aesd_device.lock); // should set the mutex to 1 ( allow decrement and access at the beginning )
    /* setup the circular buffer */
    aesd_circular_buffer_init(&aesd_device.c_buffer);

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);
    
    cdev_del(&aesd_device.cdev);
    
    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */
    mutex_lock(&aesd_device.lock);
    aesd_trim(&aesd_device); /* trim file to size 0*/
    kfree(aesd_device.partial_buffer.buffptr);
    mutex_unlock(&aesd_device.lock);
    mutex_destroy(&aesd_device.lock);
    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

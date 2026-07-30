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
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("Eyas"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;
#define END_CHARACTER '\n'

int aesd_trim(struct aesd_dev * dev);
int aesd_trim(struct aesd_dev * dev)
{
    for (int i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++)
    {
        if (dev->c_buffer->entry[i].buffptr != NULL)
        {
            kfree(dev->c_buffer->entry[i].buffptr);
            dev->c_buffer->entry[i].buffptr = NULL;
            dev->c_buffer->entry[i].size = 0;

        }
    }
    dev->c_buffer->full = false;
    dev->c_buffer->in_offs = 0;
    dev->c_buffer->out_offs = 0;
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

    /* trim file size to 0 if opend with write-only */
    if ( (filp->f_flags & O_ACCMODE) == O_WRONLY)
    {
        if (mutex_lock_interruptible(&dev->lock))
            return -ERESTARTSYS;
        // aesd_trim(dev); /* trim file to size 0*/
        mutex_unlock(&dev->lock);
    }

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
    struct aesd_buffer_entry * found_entry = aesd_circular_buffer_find_entry_offset_for_fpos(dev->c_buffer, *f_pos ,&entry_offset);
    if (found_entry == NULL)
    {
        // nothing to be read offset maybe too high
        mutex_unlock(&dev->lock);
        return 0;
    }
    const int first_addres = dev->c_buffer->out_offs;
    // found the starting point for the data to be returned
    if (count + entry_offset >= found_entry->size)
    {
        count = found_entry->size - entry_offset;
    }
    // copy to user only the count - entry_offset
    int const remaining_bytes = copy_to_user(buf, found_entry->buffptr + entry_offset, count);
    if (remaining_bytes != 0)
    {
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }
    /* update file position after read is successful*/
    *f_pos = +count;
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
    int not_copied_bytes = copy_from_user(allocated_memory, buf, count);
    if (not_copied_bytes != 0)
    {
        kfree(allocated_memory);
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }
    if (dev->partial_buffer.size == 0U)
    {
        // we got an empty partial_buffer
        dev->partial_buffer.buffptr = kmalloc(count, GFP_KERNEL);
        memcpy(dev->partial_buffer.buffptr, allocated_memory,count);
        dev->partial_buffer.size = count;
        if (dev->partial_buffer.buffptr[dev->partial_buffer.size - 1] == END_CHARACTER)
        {
            struct aesd_buffer_entry * old_entry;
            if (NULL != (old_entry=aesd_circular_buffer_add_entry(dev->c_buffer, &dev->partial_buffer)))
            {
                // need to free the overwritten entry
                   kfree(old_entry);
            }
        }
    }
    else
    {
        // append the the new data
        char * new_bigger_data = kmalloc(count + dev->partial_buffer.size, GFP_KERNEL);
        if (new_bigger_data == NULL)
        {
            kfree(allocated_memory);
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }
        /* move previous partial data to the new bigger allocated memory */
        memcpy(new_bigger_data, dev->partial_buffer.buffptr, dev->partial_buffer.size);
        /* append the data by copying to the new allocated memory with an offset of size of partial buffer */
        memcpy(new_bigger_data + dev->partial_buffer.size, allocated_memory, count);
        dev->partial_buffer.size = dev->partial_buffer.size + count;
        /* free the old data */
        kfree(dev->partial_buffer.buffptr);
        /* point the partial buffer to the new bigger data */
        dev->partial_buffer.buffptr = new_bigger_data;
        /* does the data has an end character */

        if (dev->partial_buffer.buffptr[dev->partial_buffer.size - 1] == END_CHARACTER)
        {
            /* Add the ready data and reset the partial buffer */
            aesd_circular_buffer_add_entry(dev->c_buffer, &dev->partial_buffer);
            dev->partial_buffer.buffptr = NULL;
            dev->partial_buffer.size = 0U;
        }
    }
    kfree(allocated_memory);
    mutex_unlock(&dev->lock);
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
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
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
    aesd_device.c_buffer = kmalloc(sizeof(struct aesd_circular_buffer), GFP_KERNEL);
    if (aesd_device.c_buffer == NULL)
    {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "Failed to allocate memory");
        return -ENOMEM;
    }
    aesd_circular_buffer_init(aesd_device.c_buffer);

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
    unregister_chrdev_region(devno, 1);
    mutex_lock(&aesd_device.lock);

    aesd_trim(&aesd_device); /* trim file to size 0*/
    kfree(aesd_device.partial_buffer.buffptr);
    kfree(aesd_device.c_buffer);
    mutex_unlock(&aesd_device.lock);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

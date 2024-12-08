#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kfifo.h>
#include <linux/timer.h>
#include <linux/slab.h>
#include "pchar_ioctl.h"

#define MAX_DEVICES 4
#define FIFO_SIZE 32  // Size of FIFO for each device

// Device structure for each instance
struct pchar_dev 
{
    struct cdev cdev;         
    struct kfifo mybuf;         
    dev_t devno;               
    struct timer_list timer;   
    bool timer_running;         
};

// Global variables
static struct pchar_dev *pchar_devices[MAX_DEVICES];
static int major = 250;                                
static struct class *pchar_class;                     

static void fifo_timer_callback(struct timer_list *t)
 {
    struct pchar_dev *dev = from_timer(dev, t, timer);
    unsigned char ch;
    int ret;

    // Remove one character from the FIFO and print it in the log
    ret = kfifo_get(&dev->mybuf, &ch);
    if (ret) 
    {
        printk(KERN_INFO "%s: Character '%c' removed from FIFO of device %d.\n", THIS_MODULE->name, ch, MINOR(dev->devno));
    } else
     {
        printk(KERN_INFO "%s: FIFO is empty for device %d. Stopping the timer.\n", THIS_MODULE->name, MINOR(dev->devno));
        del_timer_sync(&dev->timer);  // Stop the timer if the FIFO is empty
        dev->timer_running = false;
    }

    // Restart the timer for 1 second later
    if (dev->timer_running)
        mod_timer(&dev->timer, jiffies + msecs_to_jiffies(1000));
}

// Device operations
static int pchar_open(struct inode *pinode, struct file *pfile)
 {
    struct pchar_dev *dev = container_of(pinode->i_cdev, struct pchar_dev, cdev);
    pfile->private_data = dev;  
    printk(KERN_INFO "%s: pchar_open() called for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
    return 0;
}

static int pchar_close(struct inode *pinode, struct file *pfile) 
{
    printk(KERN_INFO "%s: pchar_close() called.\n", THIS_MODULE->name);
    return 0;
}

static ssize_t pchar_write(struct file *pfile, const char __user *ubuf, size_t bufsize, loff_t *pf_pos) 
{
    struct pchar_dev *dev = pfile->private_data;
    int nbytes, ret;
    ret = kfifo_from_user(&dev->mybuf, ubuf, bufsize, &nbytes);
    if (ret != 0) 
    {
        printk(KERN_ERR "%s: kfifo_from_user() failed for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
        return ret;
    }
    printk(KERN_INFO "%s: pchar_write() written %d bytes to device %d.\n", THIS_MODULE->name, nbytes, MINOR(dev->devno));
    return nbytes;
}

static ssize_t pchar_read(struct file *pfile, char __user *ubuf, size_t bufsize, loff_t *pf_pos) 
{
    struct pchar_dev *dev = pfile->private_data;
    int ret, nbytes;
    ret = kfifo_to_user(&dev->mybuf, ubuf, bufsize, &nbytes);
    if (ret != 0) 
    {
        printk(KERN_ERR "%s: kfifo_to_user() failed for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
        return ret;
    }
    printk(KERN_INFO "%s: pchar_read() read %d bytes from device %d.\n", THIS_MODULE->name, nbytes, MINOR(dev->devno));
    return nbytes;
}

static long pchar_ioctl(struct file *pfile, unsigned int cmd, unsigned long param) 
{
    struct pchar_dev *dev = pfile->private_data;
    devinfo_t info;
    int ret;

    switch (cmd)
     {
        case FIFO_CLEAR:
            kfifo_reset(&dev->mybuf); 
            printk(KERN_INFO "%s: pchar_ioctl() dev buffer is cleared for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
            return 0;

        case FIFO_GETINFO:
            info.size = kfifo_size(&dev->mybuf);
            info.len = kfifo_len(&dev->mybuf);
            info.avail = kfifo_avail(&dev->mybuf);
            ret = copy_to_user((void *)param, &info, sizeof(info));
            if (ret < 0) 
            {
                printk(KERN_ERR "%s: copy_to_user() failed for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
                return ret;
            }
            printk(KERN_INFO "%s: pchar_ioctl() read dev buffer info for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
            return 0;

        case FIFO_START_TIMER:
            if (!dev->timer_running) {
                printk(KERN_INFO "%s: Starting timer for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
                dev->timer_running = true;
                mod_timer(&dev->timer, jiffies + msecs_to_jiffies(1000));  // Start the timer for 1 second
            }
            return 0;

        case FIFO_STOP_TIMER:
            if (dev->timer_running)
             {
                printk(KERN_INFO "%s: Stopping timer for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
                del_timer_sync(&dev->timer);  // Stop the timer immediately
                dev->timer_running = false;
            }
            return 0;

        default:
            printk(KERN_ERR "%s: Invalid ioctl command for device %d.\n", THIS_MODULE->name, MINOR(dev->devno));
            return -EINVAL;
    }
}

// File operations structure
static struct file_operations pchar_fops = {
    .owner = THIS_MODULE,
    .open = pchar_open,
    .release = pchar_close,
    .write = pchar_write,
    .read = pchar_read,
    .unlocked_ioctl = pchar_ioctl
};

// Initialize the devices
static int __init pchar_init(void) 
{
    int ret, i;
    dev_t devno;
    struct pchar_dev *dev;

    printk(KERN_INFO "%s: pchar_init() called.\n", THIS_MODULE->name);

    // Allocate a range of device numbers (major number + minor numbers for multiple devices)
    ret = alloc_chrdev_region(&devno, 0, MAX_DEVICES, "pchar");
    if (ret < 0)
     {
        printk(KERN_ERR "%s: alloc_chrdev_region() failed.\n", THIS_MODULE->name);
        return ret;
    }
    major = MAJOR(devno);
    printk(KERN_INFO "%s: alloc_chrdev_region() device num: %d.\n", THIS_MODULE->name, major);

    // Create device class
    pchar_class = class_create(THIS_MODULE, "pchar_class");
    if (IS_ERR(pchar_class)) 
    {
        printk(KERN_ERR "%s: class_create() failed.\n", THIS_MODULE->name);
        unregister_chrdev_region(devno, MAX_DEVICES);
        return PTR_ERR(pchar_class);
    }

    // Allocate memory for each device, initialize, and register
    for (i = 0; i < MAX_DEVICES; i++)
     {
        dev = kzalloc(sizeof(struct pchar_dev), GFP_KERNEL);
        if (!dev) {
            printk(KERN_ERR "%s: kzalloc() failed for device %d.\n", THIS_MODULE->name, i);
            ret = -ENOMEM;
            goto err_cleanup;
        }

        // Allocate FIFO buffer for each device
        ret = kfifo_alloc(&dev->mybuf, FIFO_SIZE, GFP_KERNEL);
        if (ret) {
            printk(KERN_ERR "%s: kfifo_alloc() failed for device %d.\n", THIS_MODULE->name, i);
            kfree(dev);
            goto err_cleanup;
        }

        // Initialize the cdev structure
        dev->devno = MKDEV(major, i);
        cdev_init(&dev->cdev, &pchar_fops);
        dev->cdev.owner = THIS_MODULE;
        ret = cdev_add(&dev->cdev, dev->devno, 1);
        if (ret)
         {
            printk(KERN_ERR "%s: cdev_add() failed for device %d.\n", THIS_MODULE->name, i);
            kfifo_free(&dev->mybuf);
            kfree(dev);
            goto err_cleanup;
        }

        // Create device file
        if (IS_ERR(device_create(pchar_class, NULL, dev->devno, NULL, "pchar%d", i))) 
        {
            printk(KERN_ERR "%s: device_create() failed for device %d.\n", THIS_MODULE->name, i);
            cdev_del(&dev->cdev);
            kfifo_free(&dev->mybuf);
            kfree(dev);
            goto err_cleanup;
        }

        // Initialize and set up the timer
        timer_setup(&dev->timer, fifo_timer_callback, 0);
        dev->timer_running = false;

        // Store the device pointer in the global array
        pchar_devices[i] = dev;
    }

    return 0;

err_cleanup:
    for (i = 0; i < MAX_DEVICES; i++)
     {
        if (pchar_devices[i]) {
            device_destroy(pchar_class, pchar_devices[i]->devno);
            cdev_del(&pchar_devices[i]->cdev);
            kfifo_free(&pchar_devices[i]->mybuf);
            kfree(pchar_devices[i]);
        }
    }
    class_destroy(pchar_class);
    unregister_chrdev_region(MKDEV(major, 0), MAX_DEVICES);
    return ret;
}

// Cleanup
static void __exit pchar_exit(void) 
{
    int i;

    printk(KERN_INFO "%s: pchar_exit() called.\n", THIS_MODULE->name);

    // Cleanup each device
    for (i = 0; i < MAX_DEVICES; i++) 
    {
        if (pchar_devices[i]) 
        {
            if (pchar_devices[i]->timer_running)
                del_timer_sync(&pchar_devices[i]->timer);

            device_destroy(pchar_class, pchar_devices[i]->devno);
            cdev_del(&pchar_devices[i]->cdev);
            kfifo_free(&pchar_devices[i]->mybuf);
            kfree(pchar_devices[i]);
        }
    }

    class_destroy(pchar_class);
    unregister_chrdev_region(MKDEV(major, 0), MAX_DEVICES);
    printk(KERN_INFO "%s: pchar_exit() completed.\n", THIS_MODULE->name);
}

module_init(pchar_init);
module_exit(pchar_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Timer");
MODULE_AUTHOR("chetna sahu <chetna7726@gmail.com>");


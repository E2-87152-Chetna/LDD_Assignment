#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/kfifo.h>
#include <linux/slab.h>
#include "pchar_ioctl.h"

// pseudo char device
#define MAX 32
static struct kfifo mybuf; // FIFO buffer for our device
#define TMAX 32
static struct kfifo temp_buf;


// device number
static dev_t devno;
static int major = 250;
// device class
static struct class *pclass;
// device struct - cdev
static struct cdev pchar_cdev;
// device operations

static int pchar_open(struct inode *pinode, struct file *pfile) {
    printk(KERN_INFO "%s: pchar_open() called.\n", THIS_MODULE->name);
    return 0;
}

static int pchar_close(struct inode *pinode, struct file *pfile) {
    printk(KERN_INFO "%s: pchar_close() called.\n", THIS_MODULE->name);
    return 0;
}

static ssize_t pchar_write(struct file *pfile, const char __user *ubuf, size_t bufsize, loff_t *pf_pos) {
    int nbytes, ret;
    printk(KERN_INFO "%s: pchar_write() called.\n", THIS_MODULE->name);
    // copy data from user buf to kfifo mybuf
    ret = kfifo_from_user(&mybuf, ubuf, bufsize, &nbytes);
    if (ret != 0) {
        printk(KERN_INFO "%s: kfifo_from_user() failed.\n", THIS_MODULE->name);
        return ret;
    }
    printk(KERN_INFO "%s: pchar_write() written %d bytes to mybuf.\n", THIS_MODULE->name, nbytes);
    return nbytes;
}

static ssize_t pchar_read(struct file *pfile, char __user *ubuf, size_t bufsize, loff_t *pf_pos) {
    int ret, nbytes;
    printk(KERN_INFO "%s: pchar_read() called.\n", THIS_MODULE->name);
    
    // copy data from kfifo mybuf to user buf
    ret = kfifo_to_user(&mybuf, ubuf, bufsize, &nbytes);
    if (ret != 0) {
        printk(KERN_ERR "%s: kfifo_to_user() failed.\n", THIS_MODULE->name);
        return ret;
    }
    printk(KERN_INFO "%s: pchar_read() read %d bytes from mybuf.\n", THIS_MODULE->name, nbytes);
    return nbytes;
}

static long pchar_ioctl(struct file *pfile, unsigned int cmd, unsigned long param) 
{
    
    devinfo_t info;
    int ret;
    switch (cmd) 
    {
    case FIFO_CLEAR:
        kfifo_reset(&mybuf);
        printk(KERN_INFO "%s: pchar_ioctl() dev buffer is cleared.\n", THIS_MODULE->name);
        return 0;

    case FIFO_GETINFO:
        info.size = kfifo_size(&mybuf);
        info.len = kfifo_len(&mybuf);
        info.avail = kfifo_avail(&mybuf);
        ret = copy_to_user((void *)param, &info, sizeof(info));
        if (ret < 0) 
        {
            printk(KERN_ERR "%s: copy_to_user() failed in pchar_ioctl().\n", THIS_MODULE->name);
            return ret;
        }
        printk(KERN_INFO "%s: pchar_ioctl() read dev buffer info.\n", THIS_MODULE->name);
        return 0;

    case FIFREEZE: 
        // Allocate a temp array to store the current FIFO data
        int current_size = kfifo_len(&mybuf);
        
        char *temp_buf = kmalloc(current_size, GFP_KERNEL);
        if (!temp_buf)
         {
            printk(KERN_ERR "%s: kmalloc() failed for temp buffer.\n", THIS_MODULE->name);
            return -ENOMEM;
        }

        // Copy data from the current FIFO into the temp buffer
        ret = kfifo_out(&mybuf, temp_buf, current_size);
        if (ret != current_size)
         {
            printk(KERN_ERR "%s: kfifo_out() failed, could not read all data.\n", THIS_MODULE->name);
            kfree(temp_buf);
            return -EIO;
        }

        pr_info("%s: kfifo_out() created,read all data from mybuf.\n",THIS_MODULE->name);

        // Free the current FIFO buffer
        kfifo_free(&mybuf);

        // Allocate a new FIFO with the requested size (from param)
        ret = kfifo_alloc(&mybuf, param, GFP_KERNEL);
        if (ret != 0) 
        {
            printk(KERN_ERR "%s: kfifo_alloc() failed with new size %ld.\n", THIS_MODULE->name, param);
            kfree(temp_buf);
            return ret;
        }

        // Copy the data from the temp buffer back into the new FIFO
        ret = kfifo_in(&mybuf, temp_buf, current_size);
        if (ret != current_size) 
        {
            printk(KERN_ERR "%s: kfifo_in() failed, could not write all data.\n", THIS_MODULE->name);
            kfifo_free(&mybuf);
            kfree(temp_buf);
            return -EIO;
        }

        // Free the temp buffer
        kfree(temp_buf);

        printk(KERN_INFO "%s: FIFO resized to %ld bytes, data copied back.\n", THIS_MODULE->name, param);
        return 0;
    

    default:
      printk(KERN_ERR "%s: invalid command in pchar_ioctl().\n", THIS_MODULE->name);
        return -EINVAL; // invalid command
    }
}


static struct file_operations pchar_fops = {
    .owner = THIS_MODULE,
    .open = pchar_open,
    .release = pchar_close,
    .write = pchar_write,
    .read = pchar_read,
    .unlocked_ioctl = pchar_ioctl
};

static int __init pchar_init(void) 
{
    int ret;
    struct device *pdevice;

    printk(KERN_INFO "%s: pchar_init() called.\n", THIS_MODULE->name);

    // allocate device number
    devno = MKDEV(major, 0);
    ret = alloc_chrdev_region(&devno, 0, 1, "pchar");
    if (ret != 0) {
        printk(KERN_ERR "%s: alloc_chrdev_region() failed.\n", THIS_MODULE->name);
        return ret;
    }
    major = MAJOR(devno);
    printk(KERN_INFO "%s: alloc_chrdev_region() device num: %d.\n", THIS_MODULE->name, major);

    // create device class
    pclass = class_create("pchar_class");
    if (IS_ERR(pclass)) {
        printk(KERN_ERR "%s: class_create() failed.\n", THIS_MODULE->name);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO "%s: class_create() created pchar_class.\n", THIS_MODULE->name);

    // create device file
    pdevice = device_create(pclass, NULL, devno, NULL, "pchar");
    if (IS_ERR(pdevice)) {
        printk(KERN_ERR "%s: device_create() failed.\n", THIS_MODULE->name);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO "%s: device_create() created pchar device.\n", THIS_MODULE->name);

    // initialize cdev object and add it in kernel
    pchar_cdev.owner = THIS_MODULE;
    cdev_init(&pchar_cdev, &pchar_fops);
    ret = cdev_add(&pchar_cdev, devno, 1);
    if (ret != 0) {
        printk(KERN_ERR "%s: cdev_add() failed.\n", THIS_MODULE->name);
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO "%s: cdev_add() added device in kernel.\n", THIS_MODULE->name);

    // allocate kfifo
    ret = kfifo_alloc(&mybuf, MAX, GFP_KERNEL);
    if (ret != 0) {
        printk(KERN_ERR "%s: kfifo_alloc() failed.\n", THIS_MODULE->name);
        cdev_del(&pchar_cdev);
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return ret;
    }
    printk(KERN_INFO "%s: kfifo_alloc() allocated fifo of size %d.\n", THIS_MODULE->name, kfifo_size(&mybuf));

    return 0;
}

static void __exit pchar_exit(void)
 {
    printk(KERN_INFO "%s: pchar_exit() called.\n", THIS_MODULE->name);

    // release kfifo
    kfifo_free(&mybuf);
    printk(KERN_INFO "%s: kfifo_free() released kfifo.\n", THIS_MODULE->name);

    // remove cdev object from kernel
    cdev_del(&pchar_cdev);
    printk(KERN_INFO "%s: cdev_del() removed device from kernel.\n", THIS_MODULE->name);

    // destroy device file
    device_destroy(pclass, devno);
    printk(KERN_INFO "%s: device_destroy() destroyed pchar device.\n", THIS_MODULE->name);

    // destroy device class
    class_destroy(pclass);
    printk(KERN_INFO "%s: class_destroy() destroyed pchar_class.\n", THIS_MODULE->name);

    // release device number
    unregister_chrdev_region(devno, 1);
    printk(KERN_INFO "%s: unregister_chrdev_region() released device num: %d.\n", THIS_MODULE->name, major);
}

module_init(pchar_init);
module_exit(pchar_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Pseudo Char Device Driver with FIFO Resize Operation");
MODULE_AUTHOR("chetna sahu <chetna7726@gmail.com>");

#include<linux/module.h>
#include<linux/fs.h>
#include<linux/device.h>
#include<linux/cdev.h>


static dev_t dev;
static struct cdev cdev;
static struct class *pclass;

#define MAX 32

static char mybuf[MAX];

static int pchar_open(struct inode *pinode,struct file *pfile)
{
    printk(KERN_INFO "%s: pchar_open() called PID of process using this device is %d\n",THIS_MODULE->name,current->pid);
    return 0;
}

static int pchar_close(struct inode *pinode, struct file *pfile)
{
    printk(KERN_INFO "%s: pchar_close() called\n",THIS_MODULE->name);
    return 0;
}



static ssize_t pchar_write(struct file *pfile, const char __user *ubuf, size_t bufsize, loff_t *pf_pos) {
    int max_bytes, bytes_to_write, nbytes;
    printk(KERN_INFO "%s: pchar_write() called.\n", THIS_MODULE->name);
    // find max bytes available in mybuf
    max_bytes = MAX - *pf_pos;
    // calculate num of bytes to be written (min of max bytes and user buffer size)
    bytes_to_write = (max_bytes < bufsize) ? max_bytes : bufsize;
    // if no space left in mybuf, return 0 or error
    if(bytes_to_write <= 0) {
        printk(KERN_ERR "%s: pchar_write() - mybuf is full.\n", THIS_MODULE->name);
        return -ENOSPC;
    }
    // copy bytes from user buffer to mybuf & calculate bytes copied successfully
    nbytes = bytes_to_write - copy_from_user(mybuf + *pf_pos, ubuf, bytes_to_write);
    // change file pos accordingly
    *pf_pos = *pf_pos + nbytes;
    printk(KERN_INFO "%s: pchar_write() written %d bytes to mybuf.\n", THIS_MODULE->name, nbytes);
    return nbytes;
}


static ssize_t pchar_read(struct file *pfile,char __user* ubuf,size_t bufsize,loff_t *pf_pos)
{
    int max,nbytes,bytes_to_read;
    max=MAX-*pf_pos;
    if(max>bufsize)
    {
        bytes_to_read=bufsize;
    }
    else
    {
        bytes_to_read=max;
    }
    if(bytes_to_read==0)
    {
        printk(KERN_INFO "%s: pchar_read() called\n",THIS_MODULE->name);
        return -ENOSPC;
    }
    nbytes=bytes_to_read-copy_to_user(ubuf,mybuf + *pf_pos,bytes_to_read);
    *pf_pos=*pf_pos+nbytes;
    return nbytes;
}

static loff_t pchar_lseek(struct file *pfile,loff_t offset,int origin)
{
    loff_t new_pos=0;
    switch(origin)
    {
        case 0:
            new_pos=offset;
            break;

        case 1:
            new_pos=pfile-> f_pos+offset;
            break;

        case 3:
            new_pos=MAX-offset;
            break;       
    }
    if(new_pos>MAX)
    {
        new_pos=MAX;
    }
    if(new_pos<0)
    {
        new_pos=0;
    }
    pfile->f_pos=new_pos;
    return new_pos;

}


static struct file_operations fs_ops={
    .owner=THIS_MODULE,
    .open=pchar_open,
    .release=pchar_close,
     .write=pchar_write,
    .read=pchar_read,
    .llseek=pchar_lseek
};

static int __init pchar_init(void)
{
    int major=250;
    int minor=0;

    dev=MKDEV(major,minor);
    if(alloc_chrdev_region(&dev,0,1,"pchar")<0)
    {
        return -1;
    }
    if((pclass=class_create("pchar_class"))==NULL)
    {
        unregister_chrdev_region(dev,1);
        return -1;
    }

    if(device_create(pclass,NULL,dev,NULL,"pchar")==NULL)
    {
        class_destroy(pclass);
        unregister_chrdev_region(dev,1);
        return -1;
    }

    cdev_init(&cdev,&fs_ops);
    if(cdev_add(&cdev,dev,1)==-1)
    {
        device_destroy(pclass,dev);
        class_destroy(pclass);
        unregister_chrdev_region(dev,1);
        return -1;
    }

    printk(KERN_INFO "%s : Character device driver is successfully registered\n",THIS_MODULE->name);

    return 0;
}

static void __exit pchar_exit(void)
{
    cdev_del(&cdev);
    device_destroy(pclass,dev);
    class_destroy(pclass);
    unregister_chrdev_region(dev,1);
    printk(KERN_INFO "%s: Goodbye: Character device driver successfully unregistered\n",THIS_MODULE->name);
}

module_init(pchar_init);
module_exit(pchar_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Character device driver");
MODULE_AUTHOR("chetna <chetna7726@gmail.com>");

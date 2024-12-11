#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/gpio.h>

//P1.16
#define LED_GPIO    48
    
// device number
static dev_t devno;
static int major = 250;
// device class
static struct class *pclass;
// device struct - cdev
static struct cdev bbb_gpio_cdev;
// device operations
static int bbb_gpio_open(struct inode *pinode, struct file *pfile) {
    printk(KERN_INFO "%s: bbb_gpio_open() called.\n", THIS_MODULE->name);
    return 0;
}

static int bbb_gpio_close(struct inode *pinode, struct file *pfile) {
    printk(KERN_INFO "%s: bbb_gpio_close() called.\n", THIS_MODULE->name);
    return 0;
}

static ssize_t bbb_gpio_write(struct file *pfile, const char __user *ubuf, size_t bufsize, loff_t *pf_pos) {
    char kbuf[1];
    int ret;
    // read 1 byte from user space buf
    ret = copy_from_user(kbuf, ubuf, 1);
    // do error check
    printk(KERN_INFO "%s: bbb_gpio_write() received: %c\n", THIS_MODULE->name, kbuf[0]);
    // if it is '1', then set gpio out=high; else set gpio=low
    if(kbuf[0] == '1')
        gpio_set_value(LED_GPIO, 1);
    else if(kbuf[0] == '0')
        gpio_set_value(LED_GPIO, 0);
    return bufsize;
}

static ssize_t bbb_gpio_read(struct file *pfile, char __user *ubuf, size_t bufsize, loff_t *pf_pos) {
    printk(KERN_INFO "%s: bbb_gpio_read() called.\n", THIS_MODULE->name);
    return 0;
}

static struct file_operations bbb_gpio_fops = {
    .owner = THIS_MODULE,
    .open = bbb_gpio_open,
    .release = bbb_gpio_close,
    .write = bbb_gpio_write,
    .read = bbb_gpio_read
};

static int __init bbb_gpio_init(void) {
    int ret;
    struct device *pdevice;

    printk(KERN_INFO "%s: bbb_gpio_init() called.\n", THIS_MODULE->name);
    // allocate device number
    devno = MKDEV(major, 0);
    ret = alloc_chrdev_region(&devno, 0, 1, "bbb_gpio");
    if(ret != 0) {
        printk(KERN_ERR "%s: alloc_chrdev_region() failed.\n", THIS_MODULE->name);
        return ret;
    }
    major = MAJOR(devno);
    printk(KERN_INFO "%s: alloc_chrdev_region() device num: %d.\n", THIS_MODULE->name, major);
    // create device class
  pclass = class_create(THIS_MODULE,"bbb_gpio_class");
    if(IS_ERR(pclass)) {
        printk(KERN_ERR "%s: class_create() failed.\n", THIS_MODULE->name);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO "%s: class_create() created bbb_gpio_class.\n", THIS_MODULE->name);
    // create device file
    pdevice = device_create(pclass, NULL, devno, NULL, "bbb_gpio");
    if(IS_ERR(pdevice)) {
        printk(KERN_ERR "%s: device_create() failed.\n", THIS_MODULE->name);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO "%s: device_create() created bbb_gpio device.\n", THIS_MODULE->name);
    // initialize cdev object and add it in kernel
    bbb_gpio_cdev.owner = THIS_MODULE;
    cdev_init(&bbb_gpio_cdev, &bbb_gpio_fops);
    ret = cdev_add(&bbb_gpio_cdev, devno, 1);
    if(ret != 0) {
        printk(KERN_ERR "%s: cdev_add() failed.\n", THIS_MODULE->name);
        device_destroy(pclass, devno);
        class_destroy(pclass);
        unregister_chrdev_region(devno, 1);
        return -1;
    }
    printk(KERN_INFO "%s: cdev_add() added device in kernel.\n", THIS_MODULE->name);

    // check if gpio is valid
    if(!gpio_is_valid(LED_GPIO)) {
        // do cleanup
        return -1;
    }
    // request gpio pin
    ret = gpio_request(LED_GPIO, "bbb_led");
    // do error check and cleanup

    // set gpio pin as output (intially output=0)
    ret = gpio_direction_output(LED_GPIO, 0);
    // do error check and cleanup
    return 0;
}

static void __exit bbb_gpio_exit(void) {
    printk(KERN_INFO "%s: bbb_gpio_exit() called.\n", THIS_MODULE->name);
    // free gpio device
    gpio_free(LED_GPIO);
    // remove cdev object from kernel
    cdev_del(&bbb_gpio_cdev);
    printk(KERN_INFO "%s: cdev_del() removed device from kernel.\n", THIS_MODULE->name);
    // destroy device file
    device_destroy(pclass, devno);
    printk(KERN_INFO "%s: device_destroy() destroyed bbb_gpio device.\n", THIS_MODULE->name);
    // destroy device class
    class_destroy(pclass);
    printk(KERN_INFO "%s: class_destroy() destroyed bbb_gpio_class.\n", THIS_MODULE->name);
    // release device number
    unregister_chrdev_region(devno, 1);
    printk(KERN_INFO "%s: unregister_chrdev_region() released device num: %d.\n", THIS_MODULE->name, major);
}

module_init(bbb_gpio_init);
module_exit(bbb_gpio_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("BBB GPIO Driver");
MODULE_AUTHOR("Nilesh Ghule <nilesh@sunbeaminfo.com>");


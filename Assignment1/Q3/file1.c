#include <linux/init.h>
#include <linux/module.h>

static int __init desd_init(void)
{
    printk(KERN_INFO"This is the module with two files....\n");
    return 0;
}


module_init(desd_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chetna <chetna7726@gmail.com>");
MODULE_DESCRIPTION("kernel module with 2 files");

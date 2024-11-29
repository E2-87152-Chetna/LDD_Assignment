#include <linux/init.h>
#include <linux/module.h>

static int __init desd_init(void)
{
    printk(KERN_INFO"HELLO WORLD....\n");
    return 0;
}

static void __exit desd_exit(void)
{
    printk(KERN_INFO"Exiting from module.\n");
}

module_init(desd_init);
module_exit(desd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chetna <chetna7726@gmail.com>");
MODULE_DESCRIPTION("hello kernel");

#include <linux/init.h>
#include <linux/module.h>


static void __exit desd_exit(void)
{
    printk(KERN_INFO"Exiting from module with two files...\n");
}

module_exit(desd_exit);


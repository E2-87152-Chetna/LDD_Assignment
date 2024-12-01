#include<linux/module.h>
#include<linux/kernel.h>
#include "export.h"

static int __init desd_init(void)
{
    printk(KERN_INFO "%s: desd_init() called..\n",THIS_MODULE->name);
    exported_fn();
    printk(KERN_INFO "%s: Imported symbol num=%d from export.ko\n",THIS_MODULE->name,num);
    return 0;
}

static void __exit desd_exit(void)
{
    printk(KERN_INFO "%s: desd_exit() called..\n",THIS_MODULE->name);
}

module_init(desd_init);
module_exit(desd_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Import Module");
MODULE_AUTHOR("chetna <chetna7726@gmail.com>");

#include<linux/module.h>
#include<linux/kernel.h>
#include "export.h"


int num=100;
static int __init desd_init(void)
{
   
    printk(KERN_INFO "%s: desd_init() called for module with multiple exported symbols\n",THIS_MODULE->name);
    num++;
    return 0;
}

static void __exit desd_exit(void)
{
    printk(KERN_INFO "%s: desd_exit() called for module with multiple exported symbols\n",THIS_MODULE->name);
}
void exported_fn(void)
{
    printk(KERN_INFO "%s: I am an exported function from module export.ko\n",THIS_MODULE->name);
}

module_init(desd_init);
module_exit(desd_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Export Module");
MODULE_AUTHOR("chetna <chetna7726@gmail.com>");

EXPORT_SYMBOL(exported_fn);
EXPORT_SYMBOL(num);

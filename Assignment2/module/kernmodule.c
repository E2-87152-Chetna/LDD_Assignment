#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleloader.h>
#include <linux/kmod.h>
#include <linux/utsname.h>


static int __init list_modules_init(void)
{
    struct module *mod;
    
    printk(KERN_INFO "Listing all currently loaded kernel modules:\n");
    
    for_each_module(mod) {
        printk(KERN_INFO "Module: %s, Size: %lu bytes\n", mod->name, mod->core_size);
    }

    return 0;
}

static void __exit list_modules_exit(void)
{
    printk(KERN_INFO "Exiting list modules kernel module.\n");
}

module_init(list_modules_init);
module_exit(list_modules_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chetna sahu <chetna7726@gmail.com>");
MODULE_DESCRIPTION("A simple kernel module to display all loaded kernel modules.");


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>

static int __init desd_init(void)
{
    struct module *mod;
    
    pr_info("%s: desd_init() called.\n",THIS_MODULE->name);

    //loaded modules
    list_for_each_entry(mod, &module, list) {
        pr_info("%s: Loaded module list=%s\n",THIS_MODULE->name, mod->list);
    }

    return 0;
}

static void __exit desd_exit(void)
{
    pr_info("%s: desd_exit() called.\n",THIS_MODULE->name);
}

module_init(desd_init);
module_exit(desd_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("chetna sahu<chetna7726@gmail.com>");
MODULE_DESCRIPTION("Display all loaded kernel modules");

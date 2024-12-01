//Module param

#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/moduleparam.h>

static char *toWhom="World";
static int howManyTimes=1;

module_param(toWhom,charp,S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
module_param_named(cnt,howManyTimes,int,0644);

static int __init moduleparam_init(void)
{
    int i;
     printk(KERN_INFO "%s: moduleparam_init() called\n",THIS_MODULE->name);
    for(i=0;i<howManyTimes;i++)
    {
        printk(KERN_INFO "Hello %s!!\n",toWhom);
    }
    return 0;
}

static void __exit moduleparam_exit(void)
{
    int i;
    printk(KERN_INFO "%s: moduleparam_exit() called\n",THIS_MODULE->name);
    for(i=0;i<howManyTimes;i++)
    {
        printk(KERN_INFO "Goodbye %s !!\n",toWhom);
    }
}

module_init(moduleparam_init);
module_exit(moduleparam_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module Param programming");
MODULE_AUTHOR("chetna <chetna7726@gmail.com>");

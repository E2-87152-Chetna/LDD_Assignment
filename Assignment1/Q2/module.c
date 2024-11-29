#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

static char *toWhom="World";
static int howManyTimes=1;

module_param(toWhom,charp,S_IRUGO);
module_param(howManyTimes,int,S_IRUGO);
//module_param(toWhom,charp,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
//module_param(howManyTimes,int,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

static int desd_init(void)
{
    int i;
    for(i=0;i<howManyTimes;i++)
    printk(KERN_INFO"HELLO %s!!!\n",toWhom);
    return 0;
}

static void desd_exit(void)
{
    int i;

    for(i=0;i<howManyTimes;i++)
    printk(KERN_INFO"Goodbye %s!!!.\n",toWhom);
}

module_init(desd_init);
module_exit(desd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chetna <chetna7726@gmail.com>");
MODULE_DESCRIPTION("kernel module");

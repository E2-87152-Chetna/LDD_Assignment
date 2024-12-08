#include<linux/module.h>
#include<linux/kthread.h>
#include<linux/delay.h>
#include<asm/delay.h>

static int print_number(void *data)
{
    int i;
    for(i=1;i<=10;i++)
    {
        pr_info("%s: kthread(%d) running %d.\n",THIS_MODULE->name,current->pid,i);
        msleep(1000);
    }
    return 0;
}

static int __init desd_init(void)
{
    struct task_struct *task;
    pr_info("%s: desd_init() called.\n",THIS_MODULE->name);

    task = kthread_run(print_number,NULL,"numthread");
    pr_info("%s: new kernel thread created%d\n",THIS_MODULE->name,task->pid);
    return 0;
}

static void __exit desd_exit(void)
{
    pr_info("%s: desd_exit() called.\n",THIS_MODULE->name);
}

module_init(desd_init);
module_exit(desd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chetna sahu <chetna7726@gmail.com>");
MODULE_DESCRIPTION("kernel thread module");
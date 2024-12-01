#include<linux/module.h>
#include<linux/list.h>
#include<linux/slab.h>
#include<linux/shed/task.>


static int __init desd_init(void)
{
    struct task_struct *trav=current;
    pr_info("%s: desd_init() called\n",THIS_MODULE->name);
    list_for_each_entry(trav,&init,task.tasks,tasks);
     pr_info("%s: pid=%d,cmd=%d\n",THIS_MODULE->name,trav->pid,trav->comm);
    return 0;
}

static void __exit desd_init(void)
{
    pr_info("%s: desd_exit() called\n",THIS_MODULE->name);
    pr_info("%s: current process pid=%d,cmd=%s\n",THIS_MODULE->name,current->pid,current->comm);
}

module_init(desd_init);
module_exit(desd_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module demo of Display Process list (Job Queue)");
MODULE_AUTHOR("chetna <chetna7726@gmail.com>");

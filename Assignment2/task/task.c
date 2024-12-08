#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>  
#include <linux/utsname.h>

static int __init process_list_init(void) 
{
    struct task_struct *task;
    
    printk(KERN_INFO "Displaying all processes starting from the current process:\n");
    for_each_process(task) 
    {
        printk(KERN_INFO "Process PID: %d, Command: %s\n", task->pid, task->comm);
    }

    return 0;
}

static void __exit process_list_exit(void) 
{
    printk(KERN_INFO "Exiting the process list module.\n");
}

module_init(process_list_init);
module_exit(process_list_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("chetna sahu <chetna7726@gmail.com>");
MODULE_DESCRIPTION("A Kernel Module to Display All Processes Starting from Current Process");


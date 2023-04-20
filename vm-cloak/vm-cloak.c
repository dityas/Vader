#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aditya Shinde");
MODULE_DESCRIPTION("Simple syscall interceptor");


static int __init start_vm_cloak(void) {

    pr_info("Starting VMCloak\r\n");
    return 0;
}


static void __exit stop_vm_cloak(void) {

    pr_info("Stopping VMCloak\r\n");
}


module_init(start_vm_cloak);
module_exit(stop_vm_cloak);

#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kprobes.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aditya Shinde");
/*
 * Obviously, change the description to something deceptive. Maybe some
 * random wireless device driver.
 */
MODULE_DESCRIPTION("Information spoofer to beat user level virt checks");


static unsigned long **syscall_table;
static unsigned long **get_syscall_table(void);

/*
 * Find the location of kallsyms_lookup_name. Then use that to locate 
 * sys_call_table and return the address.
 */
static unsigned long **get_syscall_table(void) {

    unsigned long *kallsyms_lookup_name;
    int register_ret;

    struct kprobe lookup_probe = {
        .symbol_name = "kallsyms_lookup_name",
    };
    
    register_ret = register_kprobe(&lookup_probe);
    if (register_ret < 0) {
        pr_err("Could not register lookup_probe, %d\r\n", register_ret);
        return NULL;
    }

    kallsyms_lookup_name = (unsigned long *)(const char *)lookup_probe.addr;
    pr_info("Found kallsyms_lookup_name at %p\r\n", kallsyms_lookup_name);
    unregister_kprobe(&lookup_probe);

    return NULL;
}


static int __init start_vm_cloak(void) {

    pr_info("Starting VMCloak\r\n");

    syscall_table = get_syscall_table();

    if (syscall_table == NULL) {
        pr_err("Failed to locate syscall table\r\n");
        return -1;
    }

    return 0;
}


static void __exit stop_vm_cloak(void) {

    pr_info("Stopping VMCloak\r\n");
}


module_init(start_vm_cloak);
module_exit(stop_vm_cloak);

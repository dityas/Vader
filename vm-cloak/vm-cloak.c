#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/string.h>
#include "spoofer.h"
#include "hooks.h"


static unsigned long **syscall_table;
static unsigned long **get_syscall_table(void);


/*
 * Find the location of kallsyms_lookup_name. Then use that to locate 
 * sys_call_table and return the address.
 */
static unsigned long **get_syscall_table(void) {

    /*
     * Depending on the kernel version, kallsyms_lookup_name is sometimes 
     * explicitly exported. So to avoid conflicts, we refer to it as
     * sym_lookup locally.
     */
    unsigned long (*sym_lookup)(const char*);
    int register_ret;

    struct kprobe lookup_probe = {
        .symbol_name = "kallsyms_lookup_name",
    };
    
    register_ret = register_kprobe(&lookup_probe);
    if (register_ret < 0) {
        pr_err("Could not register lookup_probe, %d\r\n", register_ret);
        return NULL;
    }

    sym_lookup = (unsigned long (*)(const char *))lookup_probe.addr;
    pr_info("Found kallsyms_lookup_name at %p\r\n", sym_lookup);
    unregister_kprobe(&lookup_probe);

    return (unsigned long **) sym_lookup("sys_call_table");
}


/*
 * Module init and exit
 */

static int __init start_vm_cloak(void) {

    pr_info("Starting VMCloak\r\n");

    syscall_table = get_syscall_table();
    if (syscall_table == NULL) {
        pr_err("Failed to locate syscall table\r\n");
        return EFAULT;
    }

    pr_info("sys_call_table found at 0x%p\r\n", syscall_table);
    hook_syscalls(syscall_table);

    return 0;
}


static void __exit stop_vm_cloak(void) {

    unhook_syscalls(syscall_table);
    pr_info("Stopping VMCloak\r\n");
}


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Aditya Shinde");
/*
 * Obviously, change the description to something deceptive. Maybe some
 * random wireless device driver.
 */
MODULE_DESCRIPTION("Information spoofer to beat user level virt checks");


module_init(start_vm_cloak);
module_exit(stop_vm_cloak);

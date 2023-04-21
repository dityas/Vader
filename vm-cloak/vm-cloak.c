#include <linux/init.h>
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

/*
 * Define KProbe for hooking to sys_call_table symbol
 */
static struct kprobe sys_call_table_probe = {
    .symbol_name = "sys_call_table",
};

/*
 * Define and pre and post probe routines for debugging
 */
static int __kprobes probe_entry(struct kprobe *kp, struct pt_regs *regs) {

    pr_info("Found %s at %p\r\n",kp->symbol_name, kp->addr);
    return 0;
}

static int __init start_vm_cloak(void) {

    int ret;
    pr_info("Starting VMCloak\r\n");

    sys_call_table_probe.pre_handler = probe_entry;
    ret = register_kprobe(&sys_call_table_probe);

#if defined(CONFIG_KPROBES)
    pr_info("Kernel compiled with kprobes\r\n");
#else
    pr_info("Kernel not compiled with kprobes\r\n");
#endif

    if (ret < 0) {
        pr_err("Failed to register sys_call_table_probe\r\n");
        return ret;
    }


    pr_info("sys_call_table_probe is at %p\r\n", sys_call_table_probe.addr);
    unregister_kprobe(&sys_call_table_probe);
    return 0;
}


static void __exit stop_vm_cloak(void) {

    pr_info("Stopping VMCloak\r\n");
}


module_init(start_vm_cloak);
module_exit(stop_vm_cloak);

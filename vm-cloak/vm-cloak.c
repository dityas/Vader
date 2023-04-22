#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kprobes.h>


/* 
 * Declare types to make everything readable
 *
 * openat_ptr points to asmlinkage long sys_openat(...)
 */
typedef asmlinkage long (*openat_ptr)(int, const char __user *, int, umode_t);

static openat_ptr kern_openat;
static asmlinkage long vm_cloak_openat(int, const char __user *, int, umode_t);


static unsigned long **syscall_table;
static unsigned long **get_syscall_table(void);

static void hook_syscalls(void);
static void hook_openat(void);

static void unhook_syscalls(void);
static void unhook_openat(void);


static asmlinkage long vm_cloak_openat(
        int fd, const char __user *fname, int flags, umode_t mode) {

    return kern_openat(fd, fname, flags, mode);
}


static void hook_openat(void) {
    /*
     * Hook the openat syscall
     */

    kern_openat = (openat_ptr) syscall_table[__NR_openat];
    syscall_table[__NR_openat] = (unsigned long *) vm_cloak_openat;
    pr_info("openat hooked");
}

static void unhook_openat(void) {
    /*
     * Hook the openat syscall
     */

    syscall_table[__NR_openat] = (unsigned long *) kern_openat;
    pr_info("openat restored");
}

static void hook_syscalls(void) {

    hook_openat();
}

static void unhook_syscalls(void) {

    unhook_openat();
}


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


static int __init start_vm_cloak(void) {

    pr_info("Starting VMCloak\r\n");

    syscall_table = get_syscall_table();
    if (syscall_table == NULL) {
        pr_err("Failed to locate syscall table\r\n");
        return EFAULT;
    }

    pr_info("sys_call_table found at 0x%p\r\n", syscall_table);
    // hook_syscalls();
    unsigned long cr0 = read_cr0();
    write_cr0(cr0);
    pr_info("cr0 is 0x%x\r\n", cr0);

    return 0;
}


static void __exit stop_vm_cloak(void) {

    // unhook_syscalls();
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

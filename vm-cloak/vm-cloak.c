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

static inline void asm_write_cr0(unsigned long);
static void remove_write_prot(void);
static void restore_write_prot(void);

static void hook_syscalls(void);
static void hook_openat(void);

static void unhook_syscalls(void);
static void unhook_openat(void);


static unsigned long OPENAT_COUNTS = 0;

static asmlinkage long vm_cloak_openat(
        int fd, const char __user *fname, int flags, umode_t mode) {

    OPENAT_COUNTS += 1;
    pr_info("opening %s\r\n", fname);
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


static inline void asm_write_cr0(unsigned long cr0) {
    /*
     * Looks like write_cr0 in version >= 5.10 does not work after
     * CPU init. So we'll go down to assembly and do it ourselves
     */
    asm volatile("mov %0, %%cr0" : "+r"(cr0) :: "memory");
}

static void restore_write_prot(void) {

    unsigned long cr0 = read_cr0();
    pr_err("CR0 value before setting is 0x%lx", cr0);
    set_bit(16, &cr0);
    asm_write_cr0(cr0);
    cr0 = read_cr0();
    pr_err("CR0 value after setting is 0x%lx", cr0);
}


static void remove_write_prot(void) {

    unsigned long cr0 = read_cr0();
    pr_err("CR0 value before clearing is 0x%lx", cr0);
    clear_bit(16, &cr0);
    asm_write_cr0(cr0);
    cr0 = read_cr0();
    pr_err("CR0 value after clearing is 0x%lx", cr0);
}


static void hook_syscalls(void) {

    remove_write_prot();
    
    hook_openat();

    restore_write_prot();
}

static void unhook_syscalls(void) {

    remove_write_prot();

    unhook_openat();

    restore_write_prot();
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
    hook_syscalls();

    return 0;
}


static void __exit stop_vm_cloak(void) {

    unhook_syscalls();
    pr_info("openat was called %ld times\r\n", OPENAT_COUNTS);
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

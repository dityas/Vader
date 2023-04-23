#include <linux/stddef.h>
#include <linux/module.h>
#include <linux/unistd.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/string.h>
/* 
 * Declare types to make everything readable
 *
 * openat_ptr points to asmlinkage long sys_openat(...)
 *
 * calling convention with struct pt_regs:
 * RDI, RSI, RDX, R10, R8, R9
 */
typedef asmlinkage long (*syscall_ptr)(const struct pt_regs *);

static syscall_ptr kern_openat;
static void check_target_file(const char __user *);
static asmlinkage long vm_cloak_openat(const struct pt_regs *);


static unsigned long **syscall_table;
static unsigned long **get_syscall_table(void);

static inline void asm_write_cr0(unsigned long);
static void remove_write_prot(void);
static void restore_write_prot(void);

static void hook_syscalls(void);
static void hook_openat(void);

static void unhook_syscalls(void);
static void unhook_openat(void);


static int OPENAT_COUNTS = 0;


/*
 * systemd-detect-virt behavior detection
 */
static const char *target_files[] = {
    "/sys/class/dmi/id/product_name",
    "/sys/class/dmi/id/sys_vendor",
    "/sys/class/dmi/id/board_vendor",
    "/sys/class/dmi/id/bios_vendor"};


static void check_target_file(const char __user *fname) {

    int slen;

    slen = (int) strlen(fname);
    pr_info("fname %s is of len %d", fname, slen);
}

/*
 * RSI will have the const char __user *filename value.
 */
static asmlinkage long vm_cloak_openat(const struct pt_regs *regs) {

    OPENAT_COUNTS += 1;
    
    check_target_file((char __user *) regs->si);
    return kern_openat(regs);
}


static void hook_openat(void) {
    /*
     * Hook the openat syscall
     */

    kern_openat = (syscall_ptr) syscall_table[__NR_openat];
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


/*
 * Write protection mechanism manipulation
 */

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


/*
 * Hooking and unhooking functions
 */

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
    hook_syscalls();

    return 0;
}


static void __exit stop_vm_cloak(void) {

    unhook_syscalls();
    pr_info("openat was called %d times\r\n", OPENAT_COUNTS);
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

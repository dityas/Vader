#include "hooks.h"
#include "spoofer.h"

/* 
 * Declare types to make everything readable
 *
 * syscall_ptr points to asmlinkage long sys_openat(...)
 *
 * calling convention with struct pt_regs:
 * RDI, RSI, RDX, R10, R8, R9
 */

syscall_ptr kern_openat;
syscall_ptr kern_read;
syscall_ptr kern_getdents64;

/*
 * read syscall
 */
void hook_read(unsigned long **syscall_table) {

    kern_read = (syscall_ptr) syscall_table[__NR_read];
    syscall_table[__NR_read] = (unsigned long *) vm_cloak_read;
    pr_info("read hooked");
}

void unhook_read(unsigned long **syscall_table) {

    syscall_table[__NR_read] = (unsigned long *) kern_read;
    pr_info("read restored");
}

asmlinkage long vm_cloak_read(const struct pt_regs *regs) {

    int fd;
    asmlinkage long orig_result;
    char fname[512];

    if (!is_comm_allowed()) {
        fd = (int) regs->di;
        if (get_target_file_from_fd(fd, fname) >= 0) {

            orig_result = kern_read(regs);
            spoof_result((char __user *) regs->si, fname, orig_result);
            return orig_result;
        }
    }

    return kern_read(regs);
}


/*
 * getdents64 syscall
 */
void hook_getdents64(unsigned long **syscall_table) {

    kern_getdents64 = (syscall_ptr) syscall_table[__NR_getdents64];
    syscall_table[__NR_getdents64] = (unsigned long *) vm_cloak_getdents64;
    pr_info("getdents64 hooked");
}

void unhook_getdents64(unsigned long **syscall_table) {

    syscall_table[__NR_getdents64] = (unsigned long *) kern_getdents64;
    pr_info("getdents64 restored");
}

asmlinkage long vm_cloak_getdents64(const struct pt_regs *regs) {

    long size;
    int fd;

    size = kern_getdents64(regs);
    fd  = regs->di;
    // debug_getdents((struct linux_dirent64 *) regs->si, size, fd);
    
    return size;
}


/*
 * open_at syscall
 */
/*
 * RSI will have the const char __user *filename value.
 */
asmlinkage long vm_cloak_openat(const struct pt_regs *regs) {

    const char* fname;

    fname = (char *) regs->si;
    if(!check_target_file_fullname(fname)) {
        char comm[TASK_COMM_LEN];
        get_task_comm(comm, current);
        pr_alert("%s is being opened by %s, %d\r\n", 
                fname, comm, (int) current->pid);
    }

    if (!strcmp("/dev", fname))
        pr_info("/dev being read");

    return kern_openat(regs);
}

void hook_openat(unsigned long **syscall_table) {
    /*
     * Hook the openat syscall
     */

    kern_openat = (syscall_ptr) syscall_table[__NR_openat];
    syscall_table[__NR_openat] = (unsigned long *) vm_cloak_openat;
    pr_info("openat hooked");
}

void unhook_openat(unsigned long **syscall_table) {
    /*
     * Hook the openat syscall
     */

    syscall_table[__NR_openat] = (unsigned long *) kern_openat;
    pr_info("openat restored");
}


/*
 * Write protection mechanism manipulation
 */

inline void asm_write_cr0(unsigned long cr0) {
    /*
     * Looks like write_cr0 in version >= 5.10 does not work after
     * CPU init. So we'll go down to assembly and do it ourselves
     */
    asm volatile("mov %0, %%cr0" : "+r"(cr0) :: "memory");
}

void restore_write_prot(void) {

    unsigned long cr0 = read_cr0();
    set_bit(16, &cr0);
    asm_write_cr0(cr0);
    cr0 = read_cr0();
}


void remove_write_prot(void) {

    unsigned long cr0 = read_cr0();
    clear_bit(16, &cr0);
    asm_write_cr0(cr0);
    cr0 = read_cr0();
}


/*
 * Hooking and unhooking functions
 */
void hook_syscalls(unsigned long **syscall_table) {

    remove_write_prot();
    
    hook_openat(syscall_table);
    hook_read(syscall_table);
    hook_getdents64(syscall_table);

    restore_write_prot();
}

void unhook_syscalls(unsigned long **syscall_table) {

    remove_write_prot();

    unhook_openat(syscall_table);
    unhook_read(syscall_table);
    unhook_getdents64(syscall_table);

    restore_write_prot();
}

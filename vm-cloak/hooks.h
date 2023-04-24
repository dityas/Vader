#ifndef HOOKS_H
#define HOOKS_H

#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/fdtable.h>
#include <linux/printk.h>

#include "spoofer.h"


inline void asm_write_cr0(unsigned long);
void remove_write_prot(void);
void restore_write_prot(void);

void hook_openat(unsigned long **);
void unhook_openat(unsigned long **);

void hook_read(unsigned long **);
void unhook_read(unsigned long **);

void hook_syscalls(unsigned long **);
void unhook_syscalls(unsigned long **);


typedef asmlinkage long (*syscall_ptr)(const struct pt_regs *);

// Hooks for openat
asmlinkage long vm_cloak_openat(const struct pt_regs *);
asmlinkage long vm_cloak_read(const struct pt_regs *);

#endif // HOOKS_H

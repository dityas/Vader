#include <linux/stddef.h>
#include <linux/kernel.h>


static asmlinkage long vm_cloak_openat(int, const char __user *, int, umode_t);

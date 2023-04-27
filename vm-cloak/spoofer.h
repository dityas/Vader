#ifndef SPOOFER_H
#define SPOOFER_H

#include <linux/stddef.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/fdtable.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/dcache.h>
#include <linux/slab.h>
#include <linux/dirent.h>
#include <linux/mm_types.h>
#include <linux/mm.h>


int             check_target_file_fullname(const char *);
int             check_target_file_name(const char *);
struct dentry   *get_path_from_dentry(struct dentry *);
bool            is_fd_target_file(int);
int             get_target_file_from_fd(int, char *);
int             get_any_file_from_fd(int, char *);
void            get_current_command(char *);
void            spoof_result(char __user *, char *, long);
bool            is_comm_allowed(void);
void            debug_getdents(struct linux_dirent64 *, int, int);

void            print_mem(void);


#endif // SPOOFER_H

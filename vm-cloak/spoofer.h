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


int check_target_file_fullname(const char *);
int check_target_file_name(const char *);
struct dentry *get_path_from_dentry(struct dentry *);
bool is_fd_target_file(int);
int get_target_file_from_fd(int, char *);
void get_current_command(char *);
void spoof_result(char __user *, char *);
bool is_comm_allowed(void);


#endif // SPOOFER_H

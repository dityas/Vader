#ifndef SPOOFER_H
#define SPOOFER_H

#include <linux/stddef.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/dcache.h>


int check_target_file(const char *);
void get_fname_from_fd(int);
void get_current_command(char *);
bool is_comm_allowed(void);

#endif // SPOOFER_H

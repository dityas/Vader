#include "spoofer.h"

/*
 * systemd-detect-virt behavior detection
 */

struct target_file {

    const char *fullname;
    const char *sysfs_name;
    const char *fname;
};


const char *spoofed_device = "Dell Inc.\r\n";

const struct target_file tfiles[] = {
    {   "/sys/class/dmi/id/product_name"        ,
        "/devices/virtual/dmi/id/product_name"  ,
        "product_name"  
    },
    {   "/sys/class/dmi/id/sys_vendor"          ,
        "/devices/virtual/dmi/id/sys_vendor"    ,
        "sys_vendor"    
    },
    {   "/sys/class/dmi/id/board_vendor"        ,
        "/devices/virtual/dmi/id/board_vendor"  ,
        "board_vendor"  
    },
    {   "/sys/class/dmi/id/bios_vendor"         ,
        "/devices/virtual/dmi/id/bios_vendor"   ,
        "bios_vendor"   
    },
    {   "/proc/cpuinfo"                         ,
        "/proc/cpuinfo"                         ,
        "cpuinfo"   
    },
    NULL
};


const char *comm_allow_list[] = {
    "rmmod",
    "systemd-journal",
    "in:imklog",
    NULL
};


int check_target_file_fullname(const char *fname) {

    int i = 0;

    while (i < 5) {
        if (!strcmp(tfiles[i].fullname, fname) || 
                !(strcmp(tfiles[i].sysfs_name, fname))) {
            return 0;
        }

        i += 1;
    }

    return 1;
}


int check_target_file_name(const char *fname) {

    int i = 0;

    while (i < 5) {
        if (!strcmp(tfiles[i].fname, fname)) {
            return 0;
        }

        i += 1;
    }

    return 1;
}


bool is_comm_allowed(void) {

    int     i = 0;
    char    comm[TASK_COMM_LEN];

    while (comm_allow_list[i] != NULL) {

        get_task_comm(comm, current);
        if (!strcmp(comm, comm_allow_list[i]))
            return true;

        i += 1;
    }

    return false;
}


struct dentry *get_path_from_dentry(struct dentry *_dentry) {

    if (_dentry != NULL) {
        pr_info("Inside %s\r\n", _dentry->d_name.name);
        return dget_parent(_dentry);
    }

    return NULL;
}


void spoof_result(char __user *user_buff, char *fname, long read_size) {

    char *buff = kmalloc(sizeof(char) * read_size, GFP_KERNEL);
    strscpy_pad(buff, spoofed_device, (size_t) read_size);

    copy_to_user(user_buff, buff, read_size);

    kfree(buff);
}


/*
 * Try and get the absolute path of an open file from the file descriptor
 */
int get_target_file_from_fd(int _fd, char *buff) {

    struct file *open_file;
    const char *fname   = NULL;
    char *full_path     = NULL;

    full_path = kmalloc(sizeof(char) * 512, GFP_KERNEL);
    if (!full_path)
        return -1;

    open_file = fcheck(_fd);

    if (open_file != NULL) {
        fname = open_file->f_path.dentry->d_name.name;

        if (!check_target_file_name(fname)) {
            full_path = dentry_path_raw(
                    open_file->f_path.dentry, full_path, 512);

            if (!check_target_file_fullname(full_path)) {
                strcpy(buff, full_path);
                return 0;
            }

        }
    }

    if (full_path != NULL)
        kfree(full_path);

    return -1;
}


int get_any_file_from_fd(int _fd, char *buff) {

    struct file *open_file;
    const char *fname   = NULL;
    char *full_path     = NULL;

    full_path = kmalloc(sizeof(char) * 512, GFP_KERNEL);
    if (!full_path)
        return -1;

    open_file = fcheck(_fd);

    if (open_file != NULL) {
        fname = open_file->f_path.dentry->d_name.name;

        full_path = dentry_path_raw(
                open_file->f_path.dentry, full_path, 512);

        strcpy(buff, full_path);
        return 0;

    }

    if (full_path != NULL)
        kfree(full_path);

    return -1;
}


bool is_fd_target_file(int _fd) {

    struct file     *open_file;
    const char      *fname = NULL;
    char            comm[TASK_COMM_LEN];
    char            *full_path;
    char            *retval;

    full_path = kmalloc(sizeof(char) * 1024, GFP_KERNEL);
    if (!full_path) {
        pr_err("Could not allocate memory\r\n");
        return false;
    }

    get_task_comm(comm, current);
    open_file = fcheck(_fd);

    if (open_file != NULL) {
        fname = open_file->f_path.dentry->d_name.name;

        if (!check_target_file_name(fname)) {
            retval = dentry_path_raw(
                    open_file->f_path.dentry, full_path, 1024);

            if (!check_target_file_fullname(retval)) {
                pr_alert("%s reading target file %s\r\n", comm, retval);
                return true;
            }
        }
    }

    kfree(full_path);
    return false;
}


void print_mem(void) {

    long                    exec_start, exec_end;
    struct mm_struct        *mm; 
    struct vm_area_struct   *vma;
    char                    *curr_exe;
    char                    buff[512];
    int i;

    pr_info("Try to access %d's mem\r\n", current->pid);

    mm          = current->mm;
    exec_start  = mm->start_code;
    exec_end    = mm->end_code;

    vma = find_vma(mm, exec_start);

    pr_info("mm start_code = 0x%lx\r\n", mm->start_code);
    pr_info("mm end_code = 0x%lx\r\n", mm->end_code);
    pr_info("vma starts at 0x%lx\r\n", vma->vm_start);

    copy_from_user(buff, (void *) exec_start, 512);

}

void debug_getdents(struct linux_dirent64 * dirent_struct, int size, int fd) {

    char                    full_name[512];
    struct linux_dirent64   *spoofed_dirent;
    struct linux_dirent64   *dir_walker;
    unsigned long           offset = 0;

    spoofed_dirent = kzalloc(size, GFP_KERNEL);
    if (spoofed_dirent == NULL)
        return;

    if (!get_any_file_from_fd(fd, full_name))
        pr_info("Full name is %s", full_name);

    copy_from_user(spoofed_dirent, dirent_struct, size);

    while (offset < size) {
        dir_walker = spoofed_dirent + offset;
        offset += dir_walker->d_reclen;
    }

    copy_to_user(dirent_struct, spoofed_dirent, size);
    kfree(spoofed_dirent);
}



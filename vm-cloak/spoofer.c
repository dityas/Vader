#include "spoofer.h"

/*
 * systemd-detect-virt behavior detection
 */

const char *tfiles[] = {
    "/sys/class/dmi/id/product_name",
    "/sys/class/dmi/id/sys_vendor",
    "/sys/class/dmi/id/board_vendor",
    "/sys/class/dmi/id/bios_vendor"
};

const char *comm_allow_list[] = {
    "rmmod",
    "systemd-journal",
    "in:imklog",
    NULL
};


int check_target_file(const char *fname) {

    int i;
    
    for (i = 0; i < 4; i++) {
        if (!strcmp(tfiles[i], fname)) {
            return 0;
        }
    }

    return 1;
}


bool is_comm_allowed(void) {

    int i = 0;
    char comm[TASK_COMM_LEN];

    while (comm_allow_list[i] != NULL) {

        get_task_comm(comm, current);
        if (!strcmp(comm, comm_allow_list[i]))
            return true;

        i += 1;
    }

    return false;
}


void get_fname_from_fd(int fd) {

    struct file *open_file;
    const char *fname = NULL;
    const char *parent = NULL;
    char comm[TASK_COMM_LEN];

    open_file = current->files->fd_array[fd];
    get_task_comm(comm, current);
    

    if (open_file != NULL) {
        fname = open_file->f_path.dentry->d_name.name;
        parent = open_file->f_path.dentry->d_parent->d_name.name;
        pr_info("%s, %d reading %s/%s", 
                comm, (int) current->pid, parent, fname);
    }
}


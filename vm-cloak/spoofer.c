#include "spoofer.h"

/*
 * systemd-detect-virt behavior detection
 */

struct target_file {

    const char *fullname;
    const char *fname;
};

const struct target_file tfiles[] = {
    { "/sys/class/dmi/id/product_name"  ,   "product_name"  },
    { "/sys/class/dmi/id/sys_vendor"    ,   "sys_vendor"    },
    { "/sys/class/dmi/id/board_vendor"  ,   "board_vendor"  },
    { "/sys/class/dmi/id/bios_vendor"   ,   "bios_vendor"   },
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
    
    while (i < 4) {
        if (!strcmp(tfiles[i].fullname, fname)) {
            return 0;
        }

        i += 1;
    }

    return 1;
}


int check_target_file_name(const char *fname) {

    int i = 0;
    
    while (i < 4) {
        if (!strcmp(tfiles[i].fname, fname)) {
            return 0;
        }

        i += 1;
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


void get_fname_from_fd(int _fd) {

    struct file *open_file;
    const char *fname = NULL;
    char comm[TASK_COMM_LEN];

    get_task_comm(comm, current);
    open_file = fcheck(_fd);
    
    if (open_file != NULL) {
        fname = open_file->f_path.dentry->d_name.name;

        if (!check_target_file_name(fname)) {
            pr_info("%s, %d reading %s\r\n", 
                    comm, (int) current->pid, fname);
        }
    }
}


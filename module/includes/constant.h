#ifndef CONSTANT_H // Fabio Marra's copyright
#define CONSTANT_H

#include <linux/fs.h>

#define ACTIVE_FIBER 1
#define INACTIVE_FIBER 0

#define NAME "fibers"    //device name
#define F_MAJOR 10     //global major number, set to 10 because this is a misc device


#define IOCTL_CONVERT _IO(F_MAJOR, 0)
#define IOCTL_SWITCH _IOW(F_MAJOR, 1, long)
#define IOCTL_CREATE _IOW(F_MAJOR, 2, void *)
#define IOCTL_FLS_ALLOC _IO(F_MAJOR, 3)
#define IOCTL_FLS_FREE _IOW(F_MAJOR, 4, void *)
#define IOCTL_FLS_GET _IOWR(F_MAJOR, 5, void *)
#define IOCTL_FLS_SET _IOW(F_MAJOR, 6, void *)

union proc_op {
	int (*proc_get_link)(struct dentry *, struct path *);
	int (*proc_show)(struct seq_file *m,
		struct pid_namespace *ns, struct pid *pid,
		struct task_struct *task);
	const char *lsm;
};



struct pid_entry
{
    const char *name;
    unsigned int len;
    umode_t mode;
    const struct inode_operations *iop;
    const struct file_operations *fop;
    union proc_op op;
};


struct fiber_info { // data to copy from user in order to create a new fiber

  void *stack;
  void (*routine)(void *);
  void *args;
};

struct fls_info{
  long index;
  void* value;
};


#endif
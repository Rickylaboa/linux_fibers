#ifndef FIBER_PROC_H // Fabio Marra's copyright
#define FIBER_PROC_H

#include<includes/constant.h>
#include<linux/proc_fs.h>
#include<includes/fiber_struct.h>
#include<linux/fs.h>
#include <linux/fs_struct.h>

#define PROCFS_MAX_SIZE		1024



union proc_op {
	int (*proc_get_link)(struct dentry *, struct path *);
	int (*proc_show)(struct seq_file *m,
		struct pid_namespace *ns, struct pid *pid,
		struct task_struct *task);
	const char *lsm;
};


struct proc_inode
{
    struct pid *pid;
    unsigned int fd;
    union proc_op op;
    struct proc_dir_entry *pde;
    struct ctl_table_header *sysctl;
    struct ctl_table *sysctl_entry;
    struct hlist_node sysctl_inodes;
    const struct proc_ns_operations *ns_ops;
    struct inode vfs_inode;
} __randomize_layout;



struct pid_entry
{
    const char *name;
    unsigned int len;
    umode_t mode;
    const struct inode_operations *iop;
    const struct file_operations *fop;
    union proc_op op;
};

#define NOD(NAME, MODE, IOP, FOP, OP) {\
	.name = (NAME),					    \
	.len  = sizeof(NAME) - 1,			 \
	.mode = MODE,					      \
	.iop  = IOP,					       \
	.fop  = FOP,					        \
	.op   = OP,					             \
}

#define LNK(NAME, get_link)					\
	NOD(NAME, (S_IFLNK | S_IRWXUGO | S_IFDIR),				\
		&proc_pid_link_inode_operations, NULL,		\
		{ .proc_get_link = get_link } )

#define DIR(NAME, MODE, iops, fops)	\
	NOD(NAME, (S_IFDIR|(MODE)), &iops, &fops, {} )
		
void proc_init(void);
void proc_end(void);
int init_proc_fiber(struct proc_dir_entry* root,int pid);
int proc_fiber_add(struct proc_dir_entry* root,long id);
int end_proc_fiber(int pid);

#endif


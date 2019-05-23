#ifndef FIBER_PROC_H // Fabio Marra's copyright
#define FIBER_PROC_H

#include<includes/constant.h>
#include<linux/proc_fs.h>
#include<includes/fiber_struct.h>
#include<linux/fs.h>
#include <linux/fs_struct.h>

#define PROCFS_MAX_SIZE		1024

void proc_lookup_names(void);
int proc_init_process(int pid);
int proc_write(struct file *file, const char *buffer, unsigned long count,void *data);
int proc_read(struct file *file, const char *buffer, unsigned long count,void *data);
int proc_open(struct file *file, const char *buffer, unsigned long count,void *data);


#endif


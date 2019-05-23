#include<includes/proc.h>
#include<linux/string.h>
#include<linux/namei.h>
#include<linux/kallsyms.h>


static int (*proc_tgid_base_readdir)(struct file*,struct dir_context*);
static struct dentry* (*proc_tgid_base_lookup)(struct inode*,struct dentry*, unsigned int);
static int (*proc_pident_readdir)(struct file*, struct dir_context*, const struct pid_entry*, unsigned int);
static struct dentry* (*proc_pident_lookup)(struct inode*, struct dentry*, const struct pid_entry*, unsigned int);



static struct file_operations* proc_tigid_base_operations;
static struct inode_operations* proc_tgid_base_inode_operations;
static struct inode_operations proc_pid_link_inode_operations;

static struct file_operations fops = {

  .owner = THIS_MODULE,      //macro for the current module
  .open = NULL,
  .read = NULL,
};

void proc_lookup_names(){
  proc_tgid_base_readdir = (void *) kallsyms_lookup_name("proc_tgid_base_readdir");
  proc_tgid_base_lookup = (void*) kallsyms_lookup_name("proc_tgid_base_lookup");
  proc_pident_readdir = (void*) kallsyms_lookup_name("proc_pident_readdir");
  proc_pident_lookup = (void*) kallsyms_lookup_name("proc_pident_lookup");
}


int proc_init_process(int pid){
    return 1;
}


int proc_write(struct file *file, const char *buffer, unsigned long count,void *data){
    return 0;
}

int proc_read(struct file *file, const char *buffer, unsigned long count,void *data){
    return 0;
}

int proc_open(struct file *file, const char *buffer, unsigned long count,void *data){
    return 0;
}
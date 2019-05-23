#include<includes/proc.h>
#include<linux/string.h>
#include<linux/namei.h>
#include<linux/kallsyms.h>

  /* Lookup of proc tigid/pident functions
  - proc_fill_cache: > fills (IF POSSIBLE) a directory entry, putting into /proc/[pid] one 
                      subfolder or one file; (TO LOOKUP)
                      
  - proc_pident_readdir: > fills all the pid_entry of some /proc/[pid], passed via
                          proc_fill_cache, incrementing the unique number identifing 
                          each single subfolder or file. 

  - proc_tgid_base_readdir: (TO HACK!) > calls proc_pident_readdir to pass all
                          the process stuff. We have to hack it to pass it also
                          the "/fibers/" subfolder. 
  - proc_pident_lookup: > search the subfolder with the passed path, returning the
                          dentry if possible.
  - proc_tgid_base_lookup: (TO HACK) > returns the array of tgid_stuff calling pident
                          lookup.  
  
  */

static unsigned long cr0;
static struct proc_dir_entry* fibers_dir;
static int (*proc_link)(struct dentry *, struct path *);
static int (*_proc_tgid_base_readdir)(struct file*,struct dir_context*);
static int (*proc_tgid_base_readdir)(struct file*,struct dir_context*);
static struct dentry* (*proc_tgid_base_lookup)(struct inode*,struct dentry*, unsigned int);
static int (*proc_pident_readdir)(struct file*, struct dir_context*, const struct pid_entry*, unsigned int);
static struct dentry* (*proc_pident_lookup)(struct inode*, struct dentry*, const struct pid_entry*, unsigned int);


static struct file_operations* proc_tgid_base_operations;
static struct inode_operations* proc_tgid_base_inode_operations;
static struct inode_operations proc_pid_link_inode_operations;

static const struct file_operations proc_fibers_operations;
static const struct inode_operations proc_fibers_inode_operations;



static inline struct proc_inode *PROC_I(const struct inode *inode)
{
    return container_of(inode, struct proc_inode, vfs_inode);
}

static inline struct pid *proc_pid(struct inode *inode)
{
    return PROC_I(inode)->pid;
}

static inline struct task_struct *get_proc_task(struct inode *inode)
{
    return get_pid_task(proc_pid(inode), PIDTYPE_PID);
}

static int get_task_fibers(struct task_struct *task, struct path *path)
{
    int pid;

    task_lock(task);
    pid = task->parent->pid;
    char buf[64];
    snprintf(buf, 64, "proc/fibers/%d", pid);
    kern_path(buf, LOOKUP_FOLLOW, path);
    task_unlock(task);
    return 0;
}


static int fibers_link(struct dentry *dentry, struct path *path)
{
    struct task_struct *task = get_proc_task(d_inode(dentry));
    int result = -ENOENT;

    if (task)
    {
        result = get_task_fibers(task, path);
        put_task_struct(task);
    }
    return result;
}

static const struct pid_entry fiber_base_stuff[] = {
  LNK("fibers",fibers_link)
  //DIR("fibers", S_IALLUGO, proc_fibers_inode_operations, proc_fibers_operations)
};


static inline void protect(void)
{
    write_cr0(cr0);
}

static inline void unprotect(void)
{
    write_cr0(cr0 & ~0x00010000);
}

static int f_proc_tgid_base_readdir(struct file* file,struct dir_context* ctx)
{
  int fiber_pident;
  int actual_pident;
  fiber_pident = proc_pident_readdir(file, ctx, fiber_base_stuff, ARRAY_SIZE(fiber_base_stuff));
  actual_pident = proc_tgid_base_readdir(file,ctx);

  return fiber_pident && actual_pident;
}

static struct dentry *f_proc_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
  struct dentry* res;
  res = proc_tgid_base_lookup(dir,dentry,flags);
  if(IS_ERR(res) && PTR_ERR(res) == -ENOENT){ // bob
    res = proc_pident_lookup(dir, dentry,fiber_base_stuff,fiber_base_stuff + ARRAY_SIZE(fiber_base_stuff));
  }
  return res;
}


void proc_init(){
  
  cr0 = read_cr0();
  fibers_dir = proc_mkdir("fibers",NULL);
  _proc_tgid_base_readdir = (void *) kallsyms_lookup_name("proc_tgid_base_readdir");
  proc_tgid_base_readdir = (void *) kallsyms_lookup_name("proc_tgid_base_readdir");
  proc_link = (void*) kallsyms_lookup_name("proc_root_link");
  proc_tgid_base_lookup = (void*) kallsyms_lookup_name("proc_tgid_base_lookup");
  proc_pident_readdir = (void*) kallsyms_lookup_name("proc_pident_readdir");
  proc_pident_lookup = (void*) kallsyms_lookup_name("proc_pident_lookup");

  /* Lookup of proc tgid operations*/
  proc_tgid_base_operations = (struct file_operations*) kallsyms_lookup_name("proc_tgid_base_operations");
  proc_tgid_base_inode_operations = (struct inode_operations*) kallsyms_lookup_name("proc_tgid_base_inode_operations");
  proc_pid_link_inode_operations = *(struct inode_operations*) kallsyms_lookup_name("proc_tgid_base_inode_operations");
  unprotect();
  proc_tgid_base_inode_operations->lookup = f_proc_lookup;
  proc_tgid_base_operations->iterate_shared = f_proc_tgid_base_readdir;
  protect();
}

void proc_end(){
  unprotect();
  proc_tgid_base_inode_operations->lookup = proc_tgid_base_lookup;
  proc_tgid_base_operations->iterate_shared = proc_tgid_base_readdir;
  protect();
}




int init_proc_fiber(struct proc_dir_entry* root, int pid){

  int ret;
  char buf[64];
  snprintf(buf,64,"%d",pid);
  root = proc_mkdir(buf,fibers_dir);
  if(!root){
    printk(KERN_ERR "%s: impossible to generate fiber proc subfolder",NAME);
    return 0; 
  }
  return 1;
}

int proc_fiber_add(struct proc_dir_entry* root, long id){
  char buf[64];
  snprintf(buf, 64, "%ld", id);
  proc_create(buf, 0444, root, &proc_fibers_operations);
  return 1;
}

int end_proc_fiber(int pid){
  
  return 1;
}

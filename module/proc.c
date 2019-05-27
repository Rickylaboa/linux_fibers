#include<includes/proc.h>
#include<linux/string.h>
#include<linux/namei.h>
#include<linux/kallsyms.h>
#include <includes/fiber_struct.h>

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
static int (*proc_link)(struct dentry *, struct path *);
static int (*_proc_tgid_base_readdir)(struct file*,struct dir_context*);
static int (*proc_tgid_base_readdir)(struct file*,struct dir_context*);
static struct dentry* (*proc_tgid_base_lookup)(struct inode*,struct dentry*, unsigned int);
static int (*proc_pident_readdir)(struct file*, struct dir_context*, const struct pid_entry*, unsigned int);
static struct dentry* (*proc_pident_lookup)(struct inode*, struct dentry*, const struct pid_entry*, unsigned int);


static struct file_operations* proc_tgid_base_operations;
static struct inode_operations* proc_tgid_base_inode_operations;
static struct inode_operations proc_pid_link_inode_operations;
static struct file_operations f_proc_ops = {
    .owner = THIS_MODULE,
    .open = generic_file_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};




static inline void protect(void)
{
    write_cr0(cr0);
}

static inline void unprotect(void)
{
    write_cr0(cr0 & ~0x00010000);
}

static inline struct proc_inode *PROC_I(const struct inode *inode)
{
	return container_of(inode, struct proc_inode, vfs_inode);
}

static inline struct pid *proc_pid(const struct inode *inode)
{
	return PROC_I(inode)->pid;
}

static inline struct task_struct *get_proc_task(const struct inode *inode)
{
	return get_pid_task(proc_pid(inode), PIDTYPE_PID);
}

static const struct inode_operations f_proc_iops;


static const struct pid_entry try[] = {
  //LNK("fibers",fibers_link)
  DIR("x", S_IRUGO | S_IXUGO, f_proc_iops, f_proc_ops),
  DIR("y", S_IRUGO | S_IXUGO, f_proc_iops, f_proc_ops),
  DIR("zs", S_IRUGO | S_IXUGO, f_proc_iops, f_proc_ops)

};


static int fibers_folder_readdir(struct file *file, struct dir_context *ctx) {
  char buf[64];
  int res, nents = 0, i=0;
  int size;
  char* pid_s; 
  char* name;
  struct pid_entry* filled;


  long unsigned int pid;
  pid_s = file->f_path.dentry->d_parent->d_iname;


  res = kstrtoul(pid_s, 10, &pid);
  if(res != 0){
    return 0;
  }
  size = number_of_fibers(pid);
  filled = kzalloc(size*sizeof(struct pid_entry),GFP_KERNEL);
  if(!filled) return 0;

  for(i=0; i < size; i++)
  {  
    res = snprintf(buf,64,"%d",i);
    name = kmalloc(res+1,GFP_KERNEL);
    memcpy(name,buf,res+1);
    filled[i].name = name;
    filled[i].len = res;
    filled[i].mode = S_IFDIR | S_IRUGO | S_IWUGO;
    filled[i].iop = &f_proc_iops;
    filled[i].fop = &f_proc_ops;
  }
  res = proc_pident_readdir(file,ctx,filled,size);
  kfree(filled);
  return res;
}




static struct dentry *fibers_folder_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags){    
  int ret;
  char buf[64];
  char* pid_s;
  long unsigned int pid;
  int res, nents = 0, i=0;
  int size;
  struct dentry* actual_d;
  struct dentry* parent_d;
  char* name;
  struct pid_entry* filled;


  actual_d = container_of(dir->i_dentry.first, struct dentry, d_u.d_alias);
  parent_d = actual_d -> d_parent;
  pid_s = parent_d -> d_iname; 


  res = kstrtoul(pid_s, 10, &pid);
  if(res != 0){
    return NULL;
  }
  
  size = number_of_fibers(pid);
  filled = kzalloc(size*sizeof(struct pid_entry),GFP_KERNEL);
  if(!filled) return NULL;

  for(i=0; i < size; i++)
  {           
    ret = snprintf(buf,64,"%d",i);
    name = kmalloc(ret+1,GFP_KERNEL);
    memcpy(name,buf,ret+1);
    filled[i].name = name;
    filled[i].len = ret;
    filled[i].mode = S_IFDIR | S_IRUGO | S_IWUGO;
    filled[i].iop = &f_proc_iops;
    filled[i].fop = &f_proc_ops;
  }
  res = proc_pident_lookup(dir, dentry,filled,size);
  /*for(i=0; i < size; i++)
  {
    kfree(filled[i].name);
  }*/
  kfree(filled);
  return res;
}


static const struct inode_operations proc_fibers_inode_operations =
{
  .lookup = fibers_folder_lookup
};

static const struct file_operations proc_fibers_operations =
{
  .read = generic_read_dir,
  .iterate_shared = fibers_folder_readdir,
  .llseek = generic_file_llseek
};

static const struct pid_entry fiber_base_stuff[] = {
  //LNK("fibers",fibers_link)
  DIR("fibers", S_IRUGO | S_IXUGO, proc_fibers_inode_operations, proc_fibers_operations)
};

static int f_proc_tgid_base_readdir(struct file* file,struct dir_context* ctx)
{
  struct inode* dir;
  struct task_struct* task;
  int pid, num_fibers;
  int fiber_pident;
  int actual_pident;
  dir = file->f_inode;
  task = get_proc_task(dir);
  pid = task->tgid;
  num_fibers = number_of_fibers(pid);
  if(num_fibers > 0){
    fiber_pident = proc_pident_readdir(file, ctx, fiber_base_stuff, ARRAY_SIZE(fiber_base_stuff));
  }else{
    fiber_pident = 0;
  }
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




int end_proc_fiber(int pid){
  
  return 1;
}

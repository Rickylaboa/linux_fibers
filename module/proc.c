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
static int (*_proc_tgid_base_readdir)(struct file*,struct dir_context*);
static int (*proc_tgid_base_readdir)(struct file*,struct dir_context*);
static struct dentry* (*proc_tgid_base_lookup)(struct inode*,struct dentry*, unsigned int);
static int (*proc_pident_readdir)(struct file*, struct dir_context*, const struct pid_entry*, unsigned int);
static struct dentry* (*proc_pident_lookup)(struct inode*, struct dentry*, const struct pid_entry*, unsigned int);
//static const struct pid_entry* tgid_base_stuff;


static struct file_operations* proc_tgid_base_operations;
static struct inode_operations* proc_tgid_base_inode_operations;
static struct inode_operations proc_pid_link_inode_operations;

static const struct file_operations proc_fibers_operations;
static const struct inode_operations proc_fibers_inode_operations;

static const struct pid_entry fiber_base_stuff[] = {
  DIR("fibers", S_IALLUGO, proc_fibers_inode_operations, proc_fibers_operations)
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
  if(!res){
    res = proc_pident_lookup(dir, dentry,fiber_base_stuff,fiber_base_stuff + ARRAY_SIZE(fiber_base_stuff));
  }
  return res;
}


void proc_init(){

  cr0 = read_cr0();
  _proc_tgid_base_readdir = (void *) kallsyms_lookup_name("proc_tgid_base_readdir");
  proc_tgid_base_readdir = (void *) kallsyms_lookup_name("proc_tgid_base_readdir");
  proc_tgid_base_lookup = (void*) kallsyms_lookup_name("proc_tgid_base_lookup");
  proc_pident_readdir = (void*) kallsyms_lookup_name("proc_pident_readdir");
  proc_pident_lookup = (void*) kallsyms_lookup_name("proc_pident_lookup");

  /* Lookup of proc tgid operations*/
  proc_tgid_base_operations = (struct file_operations*) kallsyms_lookup_name("proc_tgid_base_operations");
  proc_tgid_base_inode_operations = (struct inode_operations*) kallsyms_lookup_name("proc_tgid_base_inode_operations");
  proc_pid_link_inode_operations = *(struct inode_operations*) kallsyms_lookup_name("proc_tgid_base_inode_operations");
  unprotect();
  //proc_tgid_base_inode_operations->lookup = f_proc_lookup;
  proc_tgid_base_operations->iterate_shared = f_proc_tgid_base_readdir;
  protect();
}

void proc_end(){
  unprotect();
  //proc_tgid_base_inode_operations->lookup = proc_tgid_base_lookup;
  proc_tgid_base_operations->iterate_shared = proc_tgid_base_readdir;
  protect();
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
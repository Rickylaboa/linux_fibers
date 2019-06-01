#include<includes/proc.h>
#include<linux/string.h>
#include<linux/namei.h>
#include<linux/kallsyms.h>
#include<includes/fiber_struct.h>

  /* Lookup of proc tigid/pident functions
  - proc_fill_cache: > fills (IF POSSIBLE) a directory entry, putting into /proc/[pid] one 
                      subfolder or one file; (TO LOOKUP)
                      
  - proc_pident_readdir: > fills all the pid_entry of some /proc/[pid], passed via
                          proc_fill_cache, incrementing the unique number identifing 
                          each single subfolder or file. 

  - proc_tgid_base_readdir: > calls proc_pident_readdir to pass all
                          the process stuff, in order to get all /proc/[pid]
                          entries during an "ls" command. 
  - proc_pident_lookup: > search the subfolder with the passed path, returning the
                          dentry if possible.
  - proc_tgid_base_lookup: > calls proc_pident_lookup to pass all
                          the process stuff, in order to get the right
                          subfolder/file issuing a command "cd /proc/[pid]/x".
  - proc_pident_instantiate: > instantiates a subfolder or file of /proc/[pid].
  
  */

static unsigned long cr0;
static struct pid_entry* tgid_base_stuff;
static int (*proc_link)(struct dentry *, struct path *);
static int (*_proc_tgid_base_readdir)(struct file*, struct dir_context*);
static int (*proc_tgid_base_readdir)(struct file*, struct dir_context*);
static struct dentry* (*proc_tgid_base_lookup)(struct inode*, struct dentry*, unsigned int);
static int (*proc_pident_readdir)(struct file*, struct dir_context*, const struct pid_entry*, unsigned int);
static struct dentry* (*proc_pident_lookup)(struct inode*, struct dentry*, const struct pid_entry*, unsigned int);
//static struct dentry* (*proc_pident_instantiate)(struct inode *dir, struct dentry *dentry,struct task_struct *task, const void *ptr);
struct inode* (*proc_pid_make_inode)(struct super_block * sb, struct task_struct *task, umode_t mode);
int* (*pid_revalidate)(struct dentry *dentry, unsigned int flags);
void* (*task_dump_owner)(struct task_struct *task, umode_t mode, kuid_t *ruid, kgid_t *rgid);
void* (*security_task_to_inode)(struct task_struct *p, struct inode *inode);


static struct file_operations* proc_tgid_base_operations;
static struct inode_operations* proc_tgid_base_inode_operations;
static struct inode_operations proc_pid_link_inode_operations;
static struct dentry_operations* pid_dentry_operations;
static const struct inode_operations f_proc_iops;


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


/*  Function used as a parser to read the specific sequential
    file of a single fiber within a process.*/
static int show_fiber_file(struct seq_file* file, void* f){
  struct fiber_struct *fiber;
  long index;

  fiber = (struct fiber_struct*) file->private;
  index = fiber->index;
  seq_printf(file, "%s %ld\n", "Fiber", index); 
  if(fiber->status == ACTIVE_FIBER){
    seq_printf(file, "%s\n", "Status: active fiber"); 
  }else{
    seq_printf(file, "%s\n", "Status: inactive fiber"); 
  }
  seq_printf(file, "%s: %lx\n", "Entry Point", fiber->entry_point); 
  seq_printf(file, "%s: %d\n", "Creator Thread ID", fiber->thread_created); 
  seq_printf(file, "%s: %ld\n", "Current Activations", fiber->current_activations); 
  seq_printf(file, "%s: %d\n", "Failed Activations", atomic_read(&(fiber->failed_activations))); 
  seq_printf(file, "%s: %lld\n", "Total Execution Time",fiber->total_time); 
  return 0;
}

/*  Function to open a specific sequencial file representing
    a fiber within a process, issuing a call to single_open
    in order to attach the fiber_struct to the field "private"
    of the file and to allow any subsequent reading. */
static int fibered_file_open(struct inode *inode, struct file *file){
  int res;
  unsigned long pid, index;
  struct fiber_struct *fiber;

  res = kstrtoul(file->f_path.dentry->d_name.name, 10, &index);
  if(res != 0){
    return -1;
  }
  res = kstrtoul(file->f_path.dentry->d_parent->d_parent->d_name.name, 10, &pid);
  if(res != 0){
    return -1;
  }
  fiber = get_fiber_pid(pid, index);
  if(!fiber) printk("Null pointer\n");
  res = single_open(file, show_fiber_file, fiber);
  return res;
}

static struct file_operations f_proc_ops = {
    .owner = THIS_MODULE,
    .open = fibered_file_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};

/* Usefull stuff to the implementation of "proc_pident_instantiate" */
static void pid_update_inode(struct task_struct *task, struct inode *inode)
{
	task_dump_owner(task, inode->i_mode, &inode->i_uid, &inode->i_gid);

	inode->i_mode &= ~(S_ISUID | S_ISGID);
	security_task_to_inode(task, inode);
}

/* Implementation of proc_pident_instantiate from kernel version
    4.15.0*/
static struct dentry *proc_pident_instantiate(struct dentry *dentry,
	struct task_struct *task, const void *ptr)
{
	const struct pid_entry *p = ptr;
	struct inode *inode;
	struct proc_inode *ei;

	inode = proc_pid_make_inode(dentry->d_sb, task, p->mode);
	if (!inode)
		return ERR_PTR(-ENOENT);

	ei = PROC_I(inode);
	if (S_ISDIR(inode->i_mode))
		set_nlink(inode, 2);	/* Use getattr to fix if necessary */
	if (p->iop)
		inode->i_op = p->iop;
	if (p->fop)
		inode->i_fop = p->fop;
	ei->op = p->op;
	pid_update_inode(task, inode);
	d_set_d_op(dentry, pid_dentry_operations);
	return d_splice_alias(inode, dentry);
}


/* Function that allows to read into the directory /proc/[pid]/fibers, 
  allowing to show correctly all the fibers of the process, if present.*/
static int fibers_folder_readdir(struct file *file, struct dir_context *ctx) {
  char buf[64];
  int res, size;
  int i = 0;
  char *pid_s; 
  char *name;
  struct pid_entry *entries;


  long unsigned int pid;
  pid_s = file->f_path.dentry->d_parent->d_iname;


  res = kstrtoul(pid_s, 10, &pid);
  if(res != 0){
    return 0;
  }
  size = number_of_fibers(pid);
  entries = kzalloc(size*sizeof(struct pid_entry), GFP_KERNEL);
  if(!entries){
    return 0;
  }

  for(i = 0; i < size; i++)
  {  
    res = snprintf(buf, 64, "%d", i);
    name = kmalloc(res + 1, GFP_KERNEL);
    memcpy(name, buf, res+1);
    entries[i].name = name;
    entries[i].len = res;
    entries[i].mode = S_IFREG | S_IRUGO | S_IWUGO;
    entries[i].iop = &f_proc_iops;
    entries[i].fop = &f_proc_ops;
  }
  res = proc_pident_readdir(file, ctx, entries, size);
  for(i = 0; i < size; i++)
  {
    kfree(entries[i].name);
  }
  kfree(entries);

  return res;
}


/*  Function allowing the lookup into /proc/[pid]/fibers directory,
    if present. */
static struct dentry *fibers_folder_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags){    
  struct dentry *ret;
  char buf[64];
  char *pid_s;
  long unsigned int pid;
  int res, size;
  int i = 0;
  struct dentry *actual_d;
  struct dentry *parent_d;
  char *name;
  struct pid_entry *entries;


  actual_d = container_of(dir->i_dentry.first, struct dentry, d_u.d_alias);
  parent_d = actual_d-> d_parent;
  pid_s = parent_d-> d_iname; 


  res = kstrtoul(pid_s, 10, &pid);
  if(res != 0){
    return NULL;
  }
  
  size = number_of_fibers(pid);
  entries = kzalloc(size*sizeof(struct pid_entry), GFP_KERNEL);
  if(!entries){
    return NULL;
  }

  for(i = 0; i < size; i++)
  {           
    res = snprintf(buf, 64, "%d", i);
    name = kmalloc(res + 1, GFP_KERNEL);
    memcpy(name, buf, res+1);
    entries[i].name = name;
    entries[i].len = res;
    entries[i].mode = S_IFREG | S_IRUGO | S_IWUGO;
    entries[i].iop = &f_proc_iops;
    entries[i].fop = &f_proc_ops;
  }
  ret = proc_pident_lookup(dir, dentry,entries, size);
  for(i = 0; i < size; i++)
  {
    kfree(entries[i].name);
  }
  kfree(entries);

  return ret;
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

/* Fibers folder, to add to /proc/[pid] */
static const struct pid_entry fiber_base_stuff[] = {
  DIR("fibers", S_IRUGO | S_IXUGO, proc_fibers_inode_operations, proc_fibers_operations)
};

/* Function allowing the reading of the /proc/[pid] directory, replacing
  proc_tgid_base_readdir, in order to show also the /fibers folder if present.*/
static int f_proc_tgid_base_readdir(struct file *file, struct dir_context *ctx)
{
  struct inode *dir;
  struct task_struct *task;
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
  actual_pident = proc_tgid_base_readdir(file, ctx);

  return fiber_pident && actual_pident;
}

/* Function allowing the lookup of the /proc/[pid]/fibers directory, replacing the generic
  proc_pident_lookup into our specific lookup of the /proc/pid. */
static struct dentry *f_proc_pident_lookup(struct inode *dir, struct dentry *dentry, const struct pid_entry *p)
{
	struct task_struct *task = get_proc_task(dir);
	struct dentry *res = ERR_PTR(-ENOENT);
  int pid;
  int num_fibers;

	if (!task){
     return res;
  }
  pid = task->tgid;
  num_fibers = number_of_fibers(pid);
  if(num_fibers > 0){
    if (!memcmp(dentry->d_name.name, p->name, p->len)) {
			res = proc_pident_instantiate(dentry, task, p);
		}
  }
	put_task_struct(task);
	return res;
}

/* Function replacing proc_lookup in order to allow a correct lookup into
  the /proc/[pid]/fibers folder. */
static struct dentry *f_proc_lookup(struct inode *dir, struct dentry *dentry, unsigned int flags)
{
  struct dentry *res;
  int size;
  res = proc_tgid_base_lookup(dir, dentry, flags);
  if(res != ERR_PTR(-ENOENT)){
    return res;
  }
  res = f_proc_pident_lookup(dir, dentry, fiber_base_stuff);

  return res;
}



void proc_init(){
  
  cr0 = read_cr0();
  _proc_tgid_base_readdir = (void *) kallsyms_lookup_name("proc_tgid_base_readdir");
  proc_tgid_base_readdir = (void *) kallsyms_lookup_name("proc_tgid_base_readdir");
  proc_link = (void *) kallsyms_lookup_name("proc_root_link");
  proc_tgid_base_lookup = (void *) kallsyms_lookup_name("proc_tgid_base_lookup");
  proc_pident_readdir = (void *) kallsyms_lookup_name("proc_pident_readdir");
  proc_pident_lookup = (void *) kallsyms_lookup_name("proc_pident_lookup");
  //proc_pident_instantiate = (void*) kallsyms_lookup_name("proc_pident_instantiate");
  proc_pid_make_inode = (void *)  kallsyms_lookup_name("proc_pid_make_inode");
  task_dump_owner = (void *) kallsyms_lookup_name("task_dump_owner");
  security_task_to_inode = (void *) kallsyms_lookup_name("security_task_to_inode");

  /* Lookup of proc tgid operations*/
  tgid_base_stuff = (struct pid_entry *) kallsyms_lookup_name("tgid_base_stuff");
  pid_dentry_operations = (struct dentry_operations *) kallsyms_lookup_name("pid_dentry_operations");
  proc_tgid_base_operations = (struct file_operations *) kallsyms_lookup_name("proc_tgid_base_operations");
  proc_tgid_base_inode_operations = (struct inode_operations *) kallsyms_lookup_name("proc_tgid_base_inode_operations");
  proc_pid_link_inode_operations = *(struct inode_operations *) kallsyms_lookup_name("proc_tgid_base_inode_operations");


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
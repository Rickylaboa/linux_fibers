#ifndef FIBER_STRUCT_H // Fabio Marra's copyright
#define FIBER_STRUCT_H

#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/mm_types.h>
#include<linux/device.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/sched.h>
#include<asm/fpu/internal.h>
#include<linux/types.h>
#include<linux/tty.h>
#include<linux/version.h>
#include<asm/ptrace.h>
#include<asm/current.h>
#include<asm/processor.h>
#include<linux/syscalls.h>
#include<linux/spinlock.h> 
#include<linux/hashtable.h>
#include <linux/list.h>
#include <linux/kprobes.h>
#include "debug.h"
#include "fls.h"
#include<linux/proc_fs.h>
#include<includes/proc.h>


#define FIBER_BKT 8
#define PROCESS_BKT 15
#define THREAD_BKT 15
#define MAX_FIBERS 10
#define NAME "fibers"


struct fiber_struct{
    int pid;
    int thread_running;
    unsigned long status;
    long index;
    long max_fls_index;
    struct fls_list* free_fls_indexes;
    DECLARE_HASHTABLE(fls_table, 5);
    struct pt_regs registers;
    struct fpu fpu_registers;
};

struct fls_list
{
    long index;
    struct list_head list;
};

struct fiber_node{
    struct fiber_struct data;
    struct hlist_node list;
};

struct process_node{
    int pid;
    long index;
    struct proc_dir_entry* proc_folder;
    struct hlist_node list;
};

struct thread_node{
    int pid;
    int tid;
    long active_fiber_index; 
    struct hlist_node list;
};

struct fiber_hash{
    spinlock_t ft_lock;
    DECLARE_HASHTABLE(fiber_table, FIBER_BKT);
};

struct process_hash{
    spinlock_t pt_lock;
    DECLARE_HASHTABLE(process_table, PROCESS_BKT);
};

struct thread_hash{
    spinlock_t tt_lock;
    DECLARE_HASHTABLE(thread_table, THREAD_BKT);
};

extern struct fiber_struct* init_fiber(int status,int pid, int thread_running,long index,struct pt_regs regs);
inline struct fiber_struct* get_fiber(long index);
inline struct fiber_struct* get_fiber_pid(int pid,long index);
inline long get_new_index(void);
inline long current_fiber(void);
inline long add_fiber(struct fiber_struct* f);
inline int add_thread(int tid,long active_fiber_index);
inline int set_thread(int tid,long active_fiber_index);
inline int is_a_fiber(void);
inline int process_has_fibers(int pid);
inline int number_of_fibers(int pid);
extern int register_exit_handler(void);
extern int unregister_exit_handler(void);
int null_handler(void);
int exit_handler(void);
extern void init_hashtables(void);
extern void remove_fiber(long index);
extern void free_all_tables(void);

#endif

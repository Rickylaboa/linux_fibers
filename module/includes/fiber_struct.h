#ifndef FIBER_STRUCT_H 
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
#include<linux/list.h>
#include<linux/kprobes.h>
#include<linux/proc_fs.h>
#include<includes/proc.h>
#include<linux/ktime.h>
#include<linux/timekeeping.h>
#include<asm/atomic.h>
#include"constant.h"
#include"fls.h"

#define FIBER_BKT 8
#define PROCESS_BKT 15
#define THREAD_BKT 15
#define MAX_FIBERS 10


struct fiber_struct{

    int pid; //  process id in which the fiber is living
    int thread_created; // thread id from which the fiber was created
    int thread_running; // thread id in which the fiber is running
    unsigned long status; // ACTIVE_FIBER (1) or INACTIVE_FIBER (0), running or not
    unsigned long current_activations; // number of successful activations
    unsigned long entry_point; // entry point address of the fiber (function to execute)
    unsigned long start_time; // variable used to compute the total time
    unsigned long total_time; // total time in which the fiber has run
    atomic_t failed_activations; // number of failed activations
    long index; // the fiber index, unique within the process id
    long max_fls_index; // the max id the fiber local storage reached
    struct fls_list *free_fls_indexes; // the list of free fiber local storage indexes
    DECLARE_HASHTABLE(fls_table, 5); // the fiber local storage hashtable
    struct pt_regs registers; // CPU context of the fiber
    struct fpu fpu_registers; // FPU context of the fiber
};

struct fls_list{

    long index; // index of the fls element 
    struct list_head list;  // next element
};

struct fiber_node{

    struct fiber_struct data; // the fiber
    struct hlist_node list; // the next in the bucket
};

struct process_node{

    int pid; // the pid of the process 
    long index; // the actual index to release in case of new fiber allocation
    struct hlist_node list; // next process in the bucket
};

struct thread_node{

    int pid; // the process id in which the thread is running
    int tid; // the pid of the thread
    long active_fiber_index; // the fiber run by the thread
    struct hlist_node list; // next thread in the bucket
};

struct fiber_hash{

    spinlock_t ft_lock; // fibers hashtable spinlock
    DECLARE_HASHTABLE(fiber_table, FIBER_BKT); // fibers hashtable
};

struct process_hash{

    spinlock_t pt_lock; // processes hashtable spinlock 
    DECLARE_HASHTABLE(process_table, PROCESS_BKT); // processes hashtable
};

struct thread_hash{

    spinlock_t tt_lock; // threads hashtable spinlock
    DECLARE_HASHTABLE(thread_table, THREAD_BKT); // threads hashtable
};

extern struct fiber_struct *init_fiber(int status, int pid, int thread_running, long index, struct pt_regs regs);
inline struct fiber_struct *get_fiber(long index);
inline struct fiber_struct *get_fiber_pid(int pid, long index);
inline long get_new_index(void);
inline long current_fiber(void);
inline long add_fiber(struct fiber_struct *f);
inline void set_thread(int tid, long active_fiber_index);
inline int add_thread(int tid, long active_fiber_index);
inline int is_a_fiber(void);
inline int number_of_fibers(int pid);
extern int register_exit_handler(void);
extern int unregister_exit_handler(void);
extern void init_hashtables(void);
int null_handler(void);
int exit_handler(void);

#endif

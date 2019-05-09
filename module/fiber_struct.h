#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mm_types.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/tty.h>
#include <linux/version.h>
#include <asm/ptrace.h>
#include <asm/current.h>
#include <asm/processor.h>
#include <linux/syscalls.h>
#include <linux/spinlock.h> 
#include <linux/hashtable.h>

#define FIBER_BKT 8
#define PROCESS_BKT 15
#define THREAD_BKT 15
#define MAX_FIBERS 10
#define NAME "fibers"


struct fiber_struct{
    int status;
    int pid;
    int thread_running;
    long index;
    struct pt_regs registers;
};

struct fiber_set{
    struct fiber_struct data;
    struct hlist_node list;
};

struct process_set{
    int pid;
    long index;
    struct hlist_node list;
};

struct thread_set{
    int tid;
    long active_fiber_index; 
    struct hlist_node list;
};

struct fiber_hash{
    spinlock_t ft_lock;
    DECLARE_HASHTABLE(fiber_table,FIBER_BKT);
};

struct process_hash{
    spinlock_t pt_lock;
    DECLARE_HASHTABLE(process_table,PROCESS_BKT);
};

struct thread_hash{
    spinlock_t tt_lock;
    DECLARE_HASHTABLE(thread_table,THREAD_BKT);
};

extern long get_new_index(void);
extern int is_a_fiber(void);
extern long current_fiber(void);
extern void init_fiber_set(void);
extern struct fiber_struct* init_fiber(int status,int pid, int thread_running,long index,struct pt_regs regs);
extern long add_fiber(struct fiber_struct* f);
extern int add_thread(int tid,long active_fiber_index);
extern void remove_fiber(long index);
extern struct fiber_struct* get_fiber(long index);
extern void free_all_tables(void);
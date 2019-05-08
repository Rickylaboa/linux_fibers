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
#include <linux/spinlock.h> // TO REMOVE WHITH HASHMAP
#include <linux/hashtable.h>

#define FIBER_BKT 8
#define MAX_DIM 2400
#define NAME "fibers"


struct fiber_struct{
    int status;
    int pid;
    int thread_running;
    long index;
    struct pt_regs registers;
};

struct fiber_set
{
    struct fiber_struct data;
    struct hlist_node list;
};

struct fiber_hash{
    DECLARE_HASHTABLE(fiber_table,FIBER_BKT);
};

extern void init_fiber_set(void);
extern struct fiber_struct* init_fiber(int status,int pid, int thread_running,long index,struct pt_regs regs);
extern long add_fiber(struct fiber_struct* f);
extern void remove_fiber(long index);
extern struct fiber_struct* get_fiber(long index);
extern void free_all_table(void);
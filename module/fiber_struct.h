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

#define MAX_DIM 2400
#define NAME "fibers"

struct fiber_struct{
    int status;
    int pid;
    long index;
    struct pt_regs registers;
};

struct fiber_list{
    struct fiber_struct* data;
    struct fiber_list* next;
};

static struct fiber_list *list_f; // TO REMOVE WITH HASHMAP
static spinlock_t list_lock = __SPIN_LOCK_UNLOCKED(list_lock); // TO REMOVE WITH HASHMAP

extern struct fiber_struct* init_fiber(int status,int pid,long index,struct pt_regs regs);
extern int add_fiber(struct fiber_struct* f);
extern int remove_fiber(long index);
extern struct fiber_struct* get_fiber(long index);
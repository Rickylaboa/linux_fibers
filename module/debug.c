#include <includes/constant.h>
#include <includes/debug.h>

static spinlock_t alloc_lock;
static int allocations; 
static unsigned long fl;


void init_allocations(void)
{
    /*DEBUGGING ALLOC*/
    spin_lock_irqsave(&(alloc_lock), fl); // begin of critical section
    allocations = 0;
    spin_unlock_irqrestore(&(alloc_lock), fl); // end of critical section
}

void add_allocation(void)
{
    spin_lock_irqsave(&(alloc_lock), fl); // begin of critical section
    allocations++;
    spin_unlock_irqrestore(&(alloc_lock), fl); // end of critical section
}

void remove_allocation(void)
{
    /*DEBUGGING ALLOC*/
    spin_lock_irqsave(&(alloc_lock), fl); // begin of critical section
    allocations--;
    spin_unlock_irqrestore(&(alloc_lock), fl); // end of critical section
}

void print_allocations(void)
{
    spin_lock_irqsave(&(alloc_lock), fl); // begin of critical section
    printk(KERN_INFO "%s: unfree allocations are %d\n",NAME ,allocations);
    spin_unlock_irqrestore(&(alloc_lock), fl); // end of critical section
}
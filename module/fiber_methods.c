#include<includes/fiber_methods.h>

spinlock_t info_lock;

/* Function to create a new (inactive) fiber, initialized
    with the userspace data: ip( instruction pointer), 
    sp (stack pointer) and di. It calls fiber alloc*/
extern long fiber_create(unsigned long ip, unsigned long sp){
    
    struct pt_regs regs;
    if(unlikely(!is_a_fiber())){

        return -1; // If not a fiber, it must before issua a convert!
    } 

    regs = *task_pt_regs(current);
    regs.ip = ip;
    regs.sp = sp;

    return fiber_alloc(INACTIVE_FIBER, regs);
}

/*  Function to convert a thread to fiber (active one!), 
    it calls fiber alloc and add the current thread to
    thread hash map, with the index of the active fiber. */
extern long fiber_convert(void){

    int ret = 0;
    long fiber_index;
    struct pt_regs regs = *task_pt_regs(current);

    fiber_index = fiber_alloc(ACTIVE_FIBER, regs);
    ret = add_thread(current->pid, fiber_index);
    if(unlikely(ret < 0)){
        printk(KERN_ERR "%s: critical error, impossible to add current running thread!\n", NAME);
    }

    return fiber_index;
}

/*  Function that allows to change the fiber context (pt_regs) from the current
    fiber, retrieved with the same called function, to the next fiber, retrieved 
    by the index passed from userspace. It first copies the actual context into 
    the current fiber context, then it copies the next fiber context into the actual
    context, both via memcpy. Before returning, it sets the new active fiber for the 
    current thread. */
extern long fiber_switch(long index){
    
    bool next_status;
    unsigned long actual_time;
    unsigned long slice;
    struct pt_regs *regs;
    struct fiber_struct *curr_fiber;
    struct fiber_struct *next_fiber;
    struct fpu *curr_fpu_regs;
    struct fpu *next_fpu_regs;
    long current_index;
    current_index = current_fiber();
    regs = task_pt_regs(current);
    if(unlikely(current_index == index)){
        
        return -1;
    }
    curr_fiber = get_fiber(current_index);
    next_fiber = get_fiber(index);
    if(unlikely(!curr_fiber)){
        printk(KERN_ERR "%s: critical error, current fiber NULL\n", NAME);
        printk(KERN_ERR "%s: current fiber index: %ld\n", NAME, current_index);

        return -1;
    }
    if(unlikely(!next_fiber)){
        printk(KERN_ERR "%s: critical error, next fiber NULL\n", NAME);

        return -1;    
    }
    next_status = test_and_set_bit(INACTIVE_FIBER, &(next_fiber->status));
    if(next_status == ACTIVE_FIBER){
        atomic_inc(&(next_fiber->failed_activations));

        return -1;
    }
    // I'm sure that only one thread will reach this point, so no need for spinlocks
    test_and_clear_bit(INACTIVE_FIBER, &(curr_fiber->status));

    actual_time = current->utime;
    slice = actual_time -  curr_fiber->start_time;
    curr_fiber->total_time += slice;
    next_fiber->start_time = actual_time;

    curr_fiber->registers = *regs;
    *regs = next_fiber->registers;
    next_fiber->current_activations++;  

    curr_fpu_regs = &(curr_fiber->fpu_registers);
    next_fpu_regs = &(next_fiber->fpu_registers);

    fpu__save(curr_fpu_regs);  
    preempt_disable();
    fpu__restore(next_fpu_regs);
    preempt_enable();

    set_thread(current->pid, index);

    return 0;
}

/*  Function called by both fiber_create and fiber_convert. It gets a fresh
    index for a new fiber, calls init fiber (fiber_struct.c) and adds the
    new fiber to the fiber hash table, returning its index. */
extern long fiber_alloc(int status, struct pt_regs regs){

    struct fiber_struct *new_fiber;
    long fiber_index = get_new_index();

    if(unlikely(fiber_index < 0)){
        printk(KERN_ERR "%s: impossible to get a new index\n", NAME);

        return -1;
    }
    new_fiber = init_fiber(status, (current->tgid), (current->pid), fiber_index, regs);
    if(status == INACTIVE_FIBER){
        preempt_disable();
        fpu__initialize(&(new_fiber->fpu_registers));
        preempt_enable();
        new_fiber->start_time = 0;
        new_fiber->total_time = 0;
    }else{
        new_fiber->current_activations = 1;
        new_fiber->start_time = current->utime;
        new_fiber->total_time = 0;
    }
    add_fiber(new_fiber);

    return fiber_index;
}
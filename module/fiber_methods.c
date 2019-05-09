#include <fiber_methods.h>

extern long fiber_create(unsigned long ip, unsigned long sp, unsigned long di){
    
    struct pt_regs regs = *task_pt_regs(current);
    
    regs.ip = ip;
    regs.sp = sp;
    regs.di = di;

    return fiber_alloc(INACTIVE_FIBER, regs);
}

extern long fiber_convert(void){

    long fiber_index;
    struct pt_regs regs = *task_pt_regs(current);

    fiber_index = fiber_alloc(ACTIVE_FIBER, regs);
    add_thread(current->pid, fiber_index);
    return fiber_index;

}

extern long fiber_switch(long index){

    struct pt_regs *regs;
    struct fiber_struct *curr_fiber;
    struct fiber_struct *next_fiber;

    regs = task_pt_regs(current);
    curr_fiber = get_fiber(current_fiber());
    next_fiber = get_fiber(index);

    memcpy(&curr_fiber->registers, regs, sizeof(struct pt_regs));
    curr_fiber->status = INACTIVE_FIBER;
    memcpy(regs, &next_fiber->registers, sizeof(struct pt_regs));
    next_fiber->status = ACTIVE_FIBER;
    set_thread(current->pid,index);

    return 0;
}
extern long fiber_alloc(int status, struct pt_regs regs){

    struct fiber_struct* new_fiber;
    long fiber_index = get_new_index();

    if(fiber_index < 0){
        printk(KERN_ERR "%s: impossible to get a new index\n",NAME);
        return -1;
    }
    new_fiber = init_fiber(status, (current->parent->pid), (current->pid), fiber_index, regs);

    add_fiber(new_fiber);
    return fiber_index;
}
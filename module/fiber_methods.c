#include <fiber_methods.h>

extern long fiber_create(struct fiber_info * nfib){
    long fiber_index;
    struct fiber_struct* nf;
    struct pt_regs regs= *task_pt_regs(current);
    
    fiber_index = get_new_index();
    if(fiber_index < 0){
        printk(KERN_ERR "%s: impossible to get a new index\n",NAME);
        return -1;
    }

    nf=init_fiber(ACTIVE_FIBER,(current->parent->pid),(current->pid),fiber_index,regs);
    add_fiber(nf);
    return fiber_index;
}
extern long fiber_convert(void){
    return 0;
}

extern long fiber_switch(void){
    return 0;
}
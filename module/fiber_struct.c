#include <fiber_struct.h>

extern struct fiber_struct* init_fiber(int status,int pid,int thread_running,long index,struct pt_regs regs){
    struct fiber_struct* new_fiber =  kmalloc(sizeof(struct fiber_struct), __GFP_HIGH);
    if(new_fiber==NULL){
        printk(KERN_ERR "%s: error in kmalloc\n",NAME);
        return NULL;   
    }
    new_fiber->status=status;
    new_fiber->pid=pid;
    new_fiber->thread_running=pid_running;
    new_fiber->index=index;
    new_fiber->registers=regs;
    return new_fiber;
}

extern int add_fiber(struct fiber_struct* f){ // TO MODIFY
    return 0;
}


extern int remove_fiber(long index){ // TO MODIFY
    return 0;
}
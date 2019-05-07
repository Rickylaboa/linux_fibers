#include <fiber_struct.h>

extern struct fiber_struct* init_fiber(int status,int tid,int pid_running,long index,struct pt_regs regs){
    struct fiber_struct* new_fiber =  kmalloc(sizeof(struct fiber_struct), __GFP_HIGH);
    if(new_fiber==NULL){
        printk(KERN_ERR "%s: error in kmalloc\n",NAME);
        return NULL;   
    }
    new_fiber->status=status;
    new_fiber->tid=tid;
    new_fiber->pid_running=pid_running;
    new_fiber->index=index;
    new_fiber->registers=regs;
    return new_fiber;
}

extern int add_fiber(struct fiber_struct* f){ // TO MODIFY
}


extern int remove_fiber(long index){ // TO MODIFY

}
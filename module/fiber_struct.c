#include <fiber_struct.h>

static struct fiber_hash ft;

extern void init_fiber_set(void){
    hash_init(ft.fiber_table);
}

extern struct fiber_struct* init_fiber(int status,int pid,int thread_running,long index,struct pt_regs regs){
    struct fiber_struct* new_fiber =  kmalloc(sizeof(struct fiber_struct), __GFP_HIGH);
    if(new_fiber==NULL){
        printk(KERN_ERR "%s: error in kmalloc\n",NAME);
        return NULL;   
    }
    new_fiber->status=status;
    new_fiber->pid=pid;
    new_fiber->thread_running=thread_running;
    new_fiber->index=index;
    new_fiber->registers=regs;
    return new_fiber;
}

extern long add_fiber(struct fiber_struct *f){
    long long key;
    struct fiber_set* elem = kmalloc(sizeof(struct fiber_set), __GFP_HIGH);
    if(elem==NULL){
        printk(KERN_INFO "%s: error in kmalloc()\n",NAME);
        return -1;
    }
    elem->data =  *f;
    //elem->list;
    key = (long long) ((long long) f->index << 32) + f->pid;
    printk(KERN_INFO "%s: Adding fiber with key %ld\n",NAME,f->index); 
    hash_add_rcu(ft.fiber_table,&(elem->list),f->index);
    return 0;
}


extern void remove_fiber(long index){ // TO MODIFY
    return;
}

extern struct fiber_struct* get_fiber(long index){

    long long key;
    struct fiber_set* curr;
    key = (long long) ((long long) index << 32) + (current->parent->pid);
    /*hash_for_each_rcu(ft.fiber_table,bkt,curr,list){
        printk(KERN_INFO "%s: Iterating %d\n",NAME,curr->data);
    }*/
    hash_for_each_possible_rcu(ft.fiber_table,curr,list,index){
        printk(KERN_INFO "%s: Iterating %ld\n",NAME,curr->data.index);
        if(curr==NULL) return NULL;
        if(curr->data.index==index && curr->data.pid == current->parent->pid){
            return &curr->data;
        }
    }
    return NULL;
}
#include <fiber_struct.h>

static struct fiber_hash ft;
static struct process_hash pt;


extern void init_fiber_set(void){
    hash_init(ft.fiber_table); // INIT FIBER HASH TABLE
    hash_init(pt.process_table); // INIT PROCESS HASH TABLE
}

extern long get_new_index(void){
    int key;
    long fresh_index;
	unsigned long flags;
    struct process_set* curr;
    struct process_set* elem;
    key = current->parent->pid; // the key in pt is the pid

    fresh_index = -1;

	spin_lock_irqsave(&(pt.pt_lock), flags); // begin of cs
    hash_for_each_possible(pt.process_table,curr,list,key){
        if(curr==NULL) break;
        if(curr->pid==key){
            curr->index = curr->index +1;
            fresh_index = curr->index;
            spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
            return fresh_index;
        }
    }
    // It means this is the first fiber of the process
    fresh_index = 0;
    elem = kmalloc(sizeof(struct process_set), __GFP_HIGH);
    if(elem==NULL){
        printk(KERN_INFO "%s: error in kmalloc()\n",NAME);
        spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
        return -1;
    }
    elem->pid = key;
    elem->index = fresh_index;
    hash_add(pt.process_table,&(elem->list),key);
    spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
    return fresh_index;
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
    unsigned long flags;
    struct fiber_set* elem = kmalloc(sizeof(struct fiber_set), __GFP_HIGH);
    if(elem==NULL){
        printk(KERN_INFO "%s: error in kmalloc()\n",NAME);
        return -1;
    }
    elem->data =  *f;
    key = (long long) ((long long) f->pid << MAX_FIBERS) + f->index;
	spin_lock_irqsave(&(ft.ft_lock), flags); // begin of cs
    hash_add_rcu(ft.fiber_table,&(elem->list),key);
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
    return 0;
}


extern void remove_fiber(long index){ // TO MODIFY
    return;
}

extern struct fiber_struct* get_fiber(long index){

    long long key;
    unsigned long flags;
    struct fiber_set* curr;
    key = (long long) ((long long) current->parent->pid << MAX_FIBERS) + index; 
	spin_lock_irqsave(&(ft.ft_lock), flags); // begin of cs
    hash_for_each_possible(ft.fiber_table,curr,list,key){
        printk(KERN_INFO "%s: Iterating FT %ld\n",NAME,curr->data.index);
        if(curr==NULL) break;
        if(curr->data.index==index && curr->data.pid == current->parent->pid){
            spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
            return &curr->data;
        }
    }
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
    return NULL;
}

extern void free_all_tables(void){
    struct fiber_set* curr1;
    struct process_set* curr2;
    int bkt1,bkt2;
    unsigned long flags;
    int i;
    i=0;

    spin_lock_irqsave(&(ft.ft_lock), flags); // begin of cs
    hash_for_each(ft.fiber_table,bkt1,curr1,list){
        kfree(curr1);
        i++;
    }  
    spin_unlock_irqrestore(&(ft.ft_lock), flags); // end of cs
    printk(KERN_INFO "%s: FT has issued kfree on %d elements \n",NAME,i);
    i=0;


    spin_lock_irqsave(&(pt.pt_lock), flags); // begin of cs
    hash_for_each(pt.process_table,bkt2,curr2,list){
        kfree(curr2);
        i++;
    }  
    spin_unlock_irqrestore(&(pt.pt_lock), flags); // end of cs
    printk(KERN_INFO "%s: PT has issued kfree on %d elements \n",NAME,i);

}